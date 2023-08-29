//
//	ImGuiProfilerRenderer.cpp
//	Fluid
//
//	Created by Diego Revilla on 06/04/22
//	Copyright � 2022 Digipen. All Rights reserved
//

#ifdef _DEBUG
#include "ImGuiProfilerRenderer.h"

// ------------------------------------------------------------------------
/*! Render
*
*   Renders the Profiler
*/ //----------------------------------------------------------------------
void ImGuiUtils::ProfilersWindow::Render() {
    fpsFramesCount++;
    auto currFrameTime = std::chrono::system_clock::now();
    {
        float fpsDeltaTime = std::chrono::duration<float>(currFrameTime - prevFpsFrameTime).count();
        if (fpsDeltaTime > 0.5f)
        {
            this->avgFrameTime = fpsDeltaTime / float(fpsFramesCount);
            fpsFramesCount = 0;
            prevFpsFrameTime = currFrameTime;
        }
    }

    std::stringstream title;
    title.precision(2);
    title << std::fixed << "Fluids Profiler [" << 1.0f / avgFrameTime << "fps\t" << avgFrameTime * 1000.0f << "ms]###ProfilerWindow";
    //###AnimatedTitle
    ImGui::SetNextWindowPos({ 1080, 0 });
    ImGui::SetNextWindowSize({ 800, 740 });
    ImGui::Begin(title.str().c_str(), 0, ImGuiWindowFlags_NoScrollbar);
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
    int maxGraphHeight = 300;
    int availableGraphHeight = (int(canvasSize.y) - sizeMargin) / 2;
    int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
    int legendWidth = 200;
    int graphWidth = int(canvasSize.x) - legendWidth;
    cpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
    frameOffset = 0;
    cpuGraph.frameWidth = frameWidth;
    cpuGraph.frameSpacing = frameSpacing;
    cpuGraph.useColoredLegendText = useColoredLegendText;

    std::vector<double> tempvalues;
    std::vector<std::string> tempnames;

    //Get the values to be ploted this frame
    for (auto& x : FunctionProfile::tasks) {
        tempvalues.emplace_back(x.GetLength() * 100);
        tempnames.emplace_back(x.name.c_str());
    }

    //Press to capture this frame into the first graph
    if (ImGui::Button("Capture Frame into Graph 1")) {
        for (auto& x : FunctionProfile::idxs) {
            capturedFrames[0].second.emplace_back(x.first);
            capturedFrames[0].first.emplace_back(FunctionProfile::tasks[x.second].GetLength() * 100);
        }

        usingcapturedframes[0] = true;
    }

    ImGui::SameLine();

    //Press to capture this frame into the second graph
    if (ImGui::Button("Capture Frame into Graph 2")) {
        for (auto& x : FunctionProfile::idxs) {
            capturedFrames[1].second.emplace_back(x.first);
            capturedFrames[1].first.emplace_back(FunctionProfile::tasks[x.second].GetLength() * 100);
        }

        usingcapturedframes[1] = true;
    }

    //Press to load a backed up frame into PIE 1
    if (ImGui::Button("Load Frame into Graph 1")) {
        LoadFrameInfo(capturedFrames[0]);
        usingcapturedframes[0] = true;
    }
    ImGui::SameLine();

    //Press to load a backed up frame into PIE 2
    if (ImGui::Button("Load Frame into Graph 2")) {
        LoadFrameInfo(capturedFrames[1]);
        usingcapturedframes[1] = true;
    }

    //Press to save the plotted frame from PIE 1
    if (ImGui::Button("Save Frame from Graph 1")) {
        if (usingcapturedframes[0])
            SerializeFrameInfo(capturedFrames[0]);
    }
    ImGui::SameLine();

    //Press to save the plotted frame from PIE 2
    if (ImGui::Button("Save Frame from Graph 2")) {
        if (usingcapturedframes[1])
            SerializeFrameInfo(capturedFrames[1]);
    }

    //Reset the PIES to plot real-time data
    if (ImGui::Button("Reset")) {
        usingcapturedframes[0] = false;
        usingcapturedframes[1] = false;
    }

    ImPlot::BeginPlot("PIE 1", { 385, 0 });

    //Plot either Real-time data or captured frames
    if (usingcapturedframes[0]) {
        ImPlot::PlotPieChart(capturedFrames[0].second.data(),
            capturedFrames[0].first.data(), capturedFrames[0].first.size(), (int)0, (int)0, (int)10, true);
    }
    else {
        ImPlot::PlotPieChart(tempnames.data(), tempvalues.data(),
            tempvalues.size(), (int)0, (int)0, (int)10, true);
    }
    ImPlot::EndPlot();
    ImGui::SameLine();
    ImPlot::BeginPlot("PIE 2", { 385, 0 });

    //Plot either Real-time data or captured frames
    if (usingcapturedframes[1]) {
        ImPlot::PlotPieChart(capturedFrames[1].second.data(),
            capturedFrames[1].first.data(), capturedFrames[1].first.size(), (int)0, (int)0, (int)10, true);
    }
    else {
        ImPlot::PlotPieChart(tempnames.data(), tempvalues.data(),
            tempvalues.size(), (int)0, (int)0, (int)10, true);
    }

    ImPlot::EndPlot();
    ImGui::End();
}

// ------------------------------------------------------------------------
/*! Serialize Frame Info
*
*   Serializes a frame information to be compared later
*/ //----------------------------------------------------------------------
void ImGuiUtils::ProfilersWindow::SerializeFrameInfo(std::pair<std::vector<double>, std::vector<std::string>>& frame) {
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Fluids Profiled Data (*.fpf)\0*.fpf\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"fpf";

    //If we succesfully chose a save route
    if (GetSaveFileName(&ofn)) {
        std::fstream file(szFileName, std::fstream::out | std::fstream::binary);

        //If we can read the file
        if (file.good()) {
            nlohmann::json j_;

            //Save the frames
            for (int i = 0; i < frame.first.size(); i++) {
                j_[i]["Name"] = frame.second[i];
                j_[i]["Duration"] = frame.first[i];
            }

            file << j_;

            file.close();
        }
    }
}

// ------------------------------------------------------------------------
/*! Load Frame Info
*
*   Loads a frame info to be compared with actual data
*/ //----------------------------------------------------------------------
void ImGuiUtils::ProfilersWindow::LoadFrameInfo(std::pair<std::vector<double>, std::vector<std::string>>& frame) {
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Fluids Profiled Data (*.fpf)\0*.fpf\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"fpf";

    //If we could find a file to open
    if (GetOpenFileName(&ofn)) {
        std::fstream file(szFileName, std::fstream::in | std::fstream::binary);

        //If we could open the file
        if (file.good()) {
            nlohmann::json j_;
            file >> j_;

            frame.first.resize(j_.size());
            frame.second.resize(j_.size());

            //Iteratively, retrieve the frame data
            for (int i = 0; i < frame.first.size(); i++) {
                frame.second[i] = j_[i]["Name"].get<std::string>();
                frame.first[i] = j_[i]["Duration"].get<double>();
            }

            file.close();
        }
    }
}

// ------------------------------------------------------------------------
/*! Conversion Constructor (ImVec2 to glm::vec2)
*
*   Constructs a glm..vec2 from an ImVec2
*/ //----------------------------------------------------------------------
glm::vec2 ImGuiUtils::Vec2(const ImVec2& vec) {
    return glm::vec2(vec.x, vec.y);
}

#endif