#include "TimingController.h"

double TimingController::s_cpuTickDelta;

void TimingController::Initialize() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	s_cpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
}

int64_t TimingController::GetCurrentTick() {
	LARGE_INTEGER currentTick;
	QueryPerformanceCounter(&currentTick);
	return static_cast<int64_t>(currentTick.QuadPart);
}

double TimingController::SecondsFromTicks(int64_t tickCount) {
	return tickCount * s_cpuTickDelta;
}

double TimingController::SecondsBetweenTicks(int64_t tick1, int64_t tick2) {
	return SecondsFromTicks(tick2 - tick1);
}
