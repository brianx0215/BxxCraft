#pragma once
#include "pch.h"

class TimingController {
public:
	static void Initialize();
	static int64_t GetCurrentTick();
	static double SecondsFromTicks(int64_t tickCount);
	static double SecondsBetweenTicks(int64_t tick1, int64_t tick2);

private:
	static double s_cpuTickDelta;
};

