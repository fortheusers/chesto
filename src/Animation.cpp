#include "Animation.hpp"
#include "./DrawUtils.hpp"

Animation::Animation(int startTime, int duration, std::function<void(float)> onStep, std::function<void()> onFinish) {
    this->startTime = startTime;
    this->duration = duration;
    this->onStep = onStep;
    this->onFinish = onFinish;
}

bool Animation::isFinished() {
    return CST_GetTicks() > startTime + duration;
}

bool Animation::step() {
    if (isFinished()) {
        onFinish();
        return true;
    }

    if (onStep != NULL) {
        float progress = (float)(CST_GetTicks() - startTime) / (float)duration;
        onStep(progress);
    }
    return false;
}