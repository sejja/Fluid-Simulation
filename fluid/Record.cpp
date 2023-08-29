//
//	Record.cpp
//	Fluid
//
//	Created by Diego Revilla on 06/04/22
//	Copyright � 2022 Digipen. All Rights reserved
//

#include "Record.h"

#ifdef _DEBUG
//Defining static member
std::unordered_map<const char*, uint16_t> FunctionProfile::idxs;
std::vector<legit::ProfilerTask> FunctionProfile::tasks;

// ------------------------------------------------------------------------
/*! Default Constructor
*
*   Called at the begginig of the code that is to be profiled
*/ //----------------------------------------------------------------------
FunctionProfile::FunctionProfile(const char* const fun) noexcept
	: mStart(std::chrono::system_clock::now()), mName(fun) {}

// ------------------------------------------------------------------------
/*! Destructor
*
*   Called at the end of the code that is being profiled
*/ //----------------------------------------------------------------------
FunctionProfile::~FunctionProfile() {
	auto it = idxs.find(mName);

	//If we have previously found it
	if (it != idxs.end()) {
		tasks[(*it).second].endTime =
			std::chrono::duration<double>(std::chrono::system_clock::now() - mStart).count();
	}
	else {
		//else, asign a track on the profiler for the new function
		static std::array<uint32_t, 17> colors = {
		legit::Colors::turqoise, legit::Colors::greenSea,
		legit::Colors::emerald, legit::Colors::nephritis,
		legit::Colors::peterRiver, legit::Colors::belizeHole,
		legit::Colors::amethyst, legit::Colors::wisteria,
		legit::Colors::sunFlower, legit::Colors::orange,
		legit::Colors::carrot, legit::Colors::pumpkin,
		legit::Colors::alizarin, legit::Colors::pomegranate,
		legit::Colors::clouds, legit::Colors::silver,
		legit::Colors::imguiText
		};
		std::uniform_int_distribution<unsigned> gen(0, 0xfffffffff);
		std::mt19937 rng(time(0) * tasks.size());

		idxs.insert({ mName, tasks.size() });
		tasks.emplace_back(legit::ProfilerTask());
		auto& place = tasks.back();
		place.name = mName;
		place.endTime = std::chrono::duration<double>(
			std::chrono::system_clock::now() - mStart).count();
		place.color = gen(rng);
	}
}

#endif