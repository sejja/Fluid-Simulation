//
//	Record.h
//	Fluid
//
//	Created by Diego Revilla on 06/04/22
//	Copyright � 2022 Digipen. All Rights reserved
//

#ifndef _RECORD__H_
#define _RECORD__H_

#ifdef _DEBUG
#include <random>
#include <ctime>
#include <array>
#include <unordered_map>
#include <chrono>
#include "ProfilerTask.h"

struct FunctionProfile {
public:
	explicit FunctionProfile(const char* const fun) noexcept;
	~FunctionProfile();

	static std::unordered_map<const char*, uint16_t> idxs;
	static std::vector <legit::ProfilerTask> tasks;

private:
	const std::chrono::system_clock::time_point mStart;
	const char* const mName;
};

#define PROFILE_CPU_USAGE	FunctionProfile __profiling(__FUNCTION__);
#else
#define PROFILE_CPU_USAGE
#endif
#endif