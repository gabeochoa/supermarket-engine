
#pragma once

#include <cmath>

#include "external_include.h"

template <typename T>
struct Interpolator {
    T min;
    T max;
    int steps;
    int step;
    T x;

    Interpolator() : steps(10) {
        x = min;
        step = 0;
    }

    Interpolator(T mn, T mx, int s) : min(mn), max(mx), steps(s) { reset(); }

    virtual T next() = 0;
    virtual void reset() {
        x = min;
        step = 0;
    }
};

struct LinearInterp : public Interpolator<float> {
    LinearInterp() : Interpolator<float>() {}
    LinearInterp(float mn, float mx, int s) : Interpolator<float>(mn, mx, s) {}

    virtual float next() override {
        float pct = fmin(1.f, 1.f * step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

#define M_SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
struct SmoothstepInterp : public Interpolator<float> {
    virtual float next() override {
        float pct = M_SMOOTHSTEP(step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

#define M_SMOOTHERSTEP(x) ((x) * (x) * (x) * ((x) * ((x)*6 - 15) + 10))
struct SmootherstepInterp : public Interpolator<float> {
    virtual float next() override {
        float pct = M_SMOOTHERSTEP(step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

struct SlowAccelInterp : public Interpolator<float> {
    virtual float next() override {
        float pct = (step / steps);
        pct = pct * pct;
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

struct SlowDecellInterp : public Interpolator<float> {
    virtual float next() override {
        float pct = (step / steps);
        pct = 1 - (1 - pct) * (1 - pct);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

struct Vec2Interpolator : public Interpolator<glm::vec2> {
    Vec2Interpolator() : Interpolator<glm::vec2>() {}
    Vec2Interpolator(glm::vec2 mn, glm::vec2 mx, int s)
        : Interpolator<glm::vec2>(mn, mx, s) {}

    virtual glm::vec2 next() override {
        float pct = fmin(1.f, 1.f * step / steps);
        x.x = (max.x * pct) + (min.x * (1 - pct));
        x.y = (max.y * pct) + (min.y * (1 - pct));
        step++;
        return x;
    }
};
