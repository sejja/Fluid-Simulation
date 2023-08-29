#pragma warning(push, 0)
#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#pragma warning(pop)

#include "Fluid.h"

#undef min
#undef max

#include "GLM/vec2.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include <array>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#ifdef _DEBUG
#include "ProfilerTask.h"
#include "ImGuiProfilerRenderer.h"
#endif
#include "Record.h"


extern Fluid* FLUID;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont* g_pFont = NULL;
ID3DXSprite* g_pSprite = NULL;
ID3DXEffect* g_pEffect = NULL;
IDirect3DTexture9* g_pTexture = NULL;
CDXUTDialogResourceManager* g_pDialogResourceManager;
CDXUTDialog* g_pHUD;
const int PARTICLE_SIZE = 10;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_RESETSIMULATION     2
#define IDC_PAUSESIMULATION     3
#define IDC_PAUSESIMULATIONONSTEP600 4
#define IDC_SIMULATIONSIZE      5
#define IDC_SIMULATIONSIZE_TEXT 6

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void InitApp();
void DestroyApp();
void RenderFluid( IDirect3DDevice9* pd3dDevice );
void RenderText( double fTime );

static IDirect3DDevice9* g_pd3dDevice = NULL;

//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
void InitializeDirectX( )
{
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCursorSettings( true, true );

    InitApp();

    DXUTInit( true, true );
    DXUTCreateWindow( L"Types of Optimization" );
    DXUTCreateDevice( true, 640, 480 );

#ifdef _DEBUG
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init(GetActiveWindow());
    ImGui_ImplDX9_Init(g_pd3dDevice);
#endif
}


//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
int RunDirectX( )
{
    DXUTMainLoop();

    DestroyApp();

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_pDialogResourceManager = new CDXUTDialogResourceManager();
    g_pHUD = new CDXUTDialog();
    g_pHUD->Init( g_pDialogResourceManager );

    // HUD
    g_pHUD->SetCallback( OnGUIEvent );
    int iY = 10;
    g_pHUD->AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_pHUD->AddButton( IDC_RESETSIMULATION, L"Reset Simulation", 35, iY += 24, 125, 22 );
    g_pHUD->AddCheckBox( IDC_PAUSESIMULATION, L"Pause Simulation", 35, iY += 24, 125, 22 );
    g_pHUD->AddCheckBox( IDC_PAUSESIMULATIONONSTEP600, L"Pause On Step 600", 35, iY += 24, 125, 22 );
    g_pHUD->AddStatic( IDC_SIMULATIONSIZE_TEXT, L"Simulation Size:", 20, iY += 24, 150, 22 );
    g_pHUD->AddSlider( IDC_SIMULATIONSIZE, 35, iY += 24, 125, 22, 1, 8, 5 );
}


//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
VOID WINAPI FillTexture( D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pTexCoord, CONST D3DXVECTOR2*, LPVOID )
{
    // Circle Texture
    float alpha = 1.0f - 2.0f * sqrtf( (pTexCoord->x - 0.5f) * (pTexCoord->x - 0.5f) + (pTexCoord->y - 0.5f) * (pTexCoord->y - 0.5f) );
    *pOut = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, alpha );
}


//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    // Turn off v-sync
    pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    return true;
}


//--------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_pDialogResourceManager->OnD3D9CreateDevice( pd3dDevice ) );

	// Create the sprite
	V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont ) );

    // Particle Texture
    V_RETURN( D3DXCreateTexture( pd3dDevice, PARTICLE_SIZE, PARTICLE_SIZE, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pTexture ) );
    V_RETURN( D3DXFillTexture( g_pTexture, FillTexture, NULL ) );
    g_pd3dDevice = pd3dDevice;

    return S_OK;
}

#ifdef _DEBUG
static ImGuiUtils::ProfilersWindow profiler;
#endif

//--------------------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0xFF, 0x80, 0x80, 0x80 ), 1.0f, 0 ) )
    {
        V( pd3dDevice->BeginScene() );

#ifdef _DEBUG
        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        profiler.cpuGraph.LoadFrameData(FunctionProfile::tasks.data(),
            FunctionProfile::tasks.size());
        profiler.Render();

        // Rendering
        ImGui::EndFrame();
        ImGui::Render();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
#endif
        // Fluid
        RenderFluid( pd3dDevice );

        // HUD
        g_pHUD->OnRender( fElapsedTime );

        // FPS and Statistics
        RenderText( fTime );
#ifdef _DEBUG
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#endif

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the Fluid
//--------------------------------------------------------------------------------------
void RenderFluid( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();

    V( g_pSprite->Begin( D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE ) );

    pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 32 );
    pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

    float screen_scale_x = pBackBufferSurfaceDesc->Width / FLUID->Width();
    float screen_scale_y = pBackBufferSurfaceDesc->Height / FLUID->Height();

    // Render each particle
    for ( unsigned int i = 0 ; i < FLUID->Size() ; i++ ) 
	{
        float screen_x = FLUID->particles[i].pos.x * screen_scale_x;
        float screen_y = FLUID->particles[i].pos.y * screen_scale_y;
        D3DXVECTOR3 position( screen_x, screen_y, 0 );
        D3DXVECTOR3 center( PARTICLE_SIZE / 2, PARTICLE_SIZE / 2, 0 );
        unsigned int color = (unsigned int)(128 * FLUID->particles[i].density / FluidRestDensity);
        color = (color > 255)? 255 : color;
        g_pSprite->Draw( g_pTexture, NULL, &center, &position, D3DCOLOR_ARGB( 255, 255 - color, 255 - color, 255 ) );
    }

    V( g_pSprite->End() );
}


//--------------------------------------------------------------------------------------
// Render the FPS and statistics text
//--------------------------------------------------------------------------------------
void RenderText( double fTime )
{
    CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawFormattedTextLine( L"Particles: %i", FLUID->Size() );
    txtHelper.DrawFormattedTextLine( L"Step: %i", FLUID->Step() );

    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Handles the input events
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_pDialogResourceManager->MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    *pbNoFurtherProcessing = g_pHUD->MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    return 0;
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_PAUSESIMULATION:
            FLUID->Pause( g_pHUD->GetCheckBox( IDC_PAUSESIMULATION )->GetChecked() ); break;
        case IDC_PAUSESIMULATIONONSTEP600:
            FLUID->PauseOnStep( (g_pHUD->GetCheckBox( IDC_PAUSESIMULATIONONSTEP600 )->GetChecked())? 600 : 0xFFFFFFFF ); break;
        case IDC_SIMULATIONSIZE:
            // fallthrough
        case IDC_RESETSIMULATION:
            FLUID->Fill( g_pHUD->GetSlider( IDC_SIMULATIONSIZE )->GetValue() / 10.0f );
            break;
    }
}


//--------------------------------------------------------------------------------------
// Reset Device
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    if( g_pDialogResourceManager )
        V_RETURN( g_pDialogResourceManager->OnD3D9ResetDevice() );
    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );
    if( g_pSprite )
        V_RETURN( g_pSprite->OnResetDevice() );

    g_pHUD->SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_pHUD->SetSize( 170, 170 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Lost Device
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    if( g_pDialogResourceManager )
        g_pDialogResourceManager->OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    if( g_pSprite )
        g_pSprite->OnLostDevice();
}


//--------------------------------------------------------------------------------------
// Cleanup 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    if( g_pDialogResourceManager )
        g_pDialogResourceManager->OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pSprite );
    SAFE_RELEASE( g_pTexture );
#ifdef _DEBUG
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif
}


//--------------------------------------------------------------------------------------
// Cleanup
//--------------------------------------------------------------------------------------
void DestroyApp()
{
    delete g_pHUD; g_pHUD = NULL;
    g_pDialogResourceManager->OnD3D9LostDevice();
    g_pDialogResourceManager->OnD3D9DestroyDevice();
    delete g_pDialogResourceManager; g_pDialogResourceManager = NULL;
}
