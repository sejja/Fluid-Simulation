
#pragma once

#pragma warning(push, 0)
#include <d3dx9math.h>
#pragma warning(pop)
#include <vector>
#include <list>
#include <memory>

// Fluid magic numbers
constexpr float FluidTimestep = 0.005f;
constexpr float FluidSmoothLen = 0.012f;
constexpr float FluidStaticStiff = 3000.0f;
constexpr float FluidRestDensity = 1000.0f;
constexpr float FluidWaterMass = 0.0002f;
constexpr float FluidViscosity = 0.1f;
constexpr float FluidStiff = 200.0f;
constexpr float FluidInitialSpacing = 0.0045f;

/*****************************************************************************/

struct FluidNeighborRecord 
{
	unsigned short p; // Particle Index
	unsigned short n; // Neighbor Index		
	float distsq; // Distance Squared and Non-Squared (After Density Calc)
};

struct FluidGridOffset 
{
	unsigned short offset; // offset into gridindices
	unsigned short count; // number of particles in cell
};

/*****************************************************************************/

struct Particle
{
	Particle();
	Particle(const D3DXVECTOR2& pos);
	D3DXVECTOR2 pos;
	D3DXVECTOR2 vel;
	D3DXVECTOR2 acc;
	float density;
	float pressure;
};

class Fluid 
{
	public:
		/* Common Interface */
		Fluid();
		~Fluid();

		void Create(const float w, const float h);
		void Fill(const float size);
		void Clear();
		void Update(const float dt) noexcept;

		/* Common Data */
		std::vector<Particle> particles;
		std::vector<std::vector<int>> mGrid;

		unsigned int neighbors_capacity;
		unsigned int num_neighbors;
		std::unique_ptr<FluidNeighborRecord> neighbors;
		
		std::size_t inline Size() const noexcept				{ return particles.size(); }
		std::size_t inline Step() const	noexcept { return step; }
		void  inline Pause( const bool p ) noexcept { paused = p; }
		void inline PauseOnStep( const unsigned p )	{ pause_step = p; }
		float inline Width()	const noexcept					{ return width; }
		float inline Height()	const noexcept					{ return height; }

	private:
		
		/* Simulation */
		void GetNeighbors() noexcept;
		void ComputeDensity() noexcept;
		void ComputeForce() noexcept;
		void Integrate(const float dt) noexcept;

	private:
		/* Run State */
		std::size_t step;
		bool paused;
		std::size_t pause_step;

		/* World Size */
		float width;
		float height;
		unsigned short grid_w;
		unsigned short grid_h;

		/* Coefficients for kernel */
		float poly6_coef;
		float grad_spiky_coef;
		float lap_vis_coef;
};
