#pragma once
#include <functional>

class Animation
{
public:
	Animation(int startTime, int duration, std::function<void(float)> onStep, std::function<void()> onFinish);

    bool isFinished();
    bool step();
    int startTime = 0;
    int duration = 0;
    std::function<void(float)> onStep = NULL;
    std::function<void()> onFinish = NULL;
};
