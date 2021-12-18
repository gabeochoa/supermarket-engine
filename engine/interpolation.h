
#pragma once

#include "pch.hpp"

struct BaseInterp {
    float min;
    float max;
    int steps;
    int step;
    float x;

    BaseInterp(float mn, float mx, int s) : min(mn), max(mx), steps(s) {
        x = min;
        step = 0;
    }

    virtual float next() = 0;
};

struct LinearInterp : public BaseInterp {
    virtual float next() override {
        float pct = step / steps;
        x = (min * pct) + (max * (1 - pct));
        return x;
    }
};

#define M_SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
struct SmoothstepInterp : public BaseInterp {
    virtual float next() override {
        float pct = M_SMOOTHSTEP(step / steps);
        x = (min * pct) + (max * (1 - pct));
        return x;
    }
};

#define M_SMOOTHERSTEP(x) ((x) * (x) * (x) * ((x) * ((x)*6 - 15) + 10))
struct SmootherstepInterp : public BaseInterp {
    virtual float next() override {
        float pct = M_SMOOTHERSTEP(step / steps);
        x = (min * pct) + (max * (1 - pct));
        return x;
    }
};

struct SlowAccelInterp : public BaseInterp {
    virtual float next() override {
        float pct = (step / steps);
        pct = pct * pct;
        x = (min * pct) + (max * (1 - pct));
        return x;
    }
};

struct SlowDecellInterp : public BaseInterp {
    virtual float next() override {
        float pct = (step / steps);
        pct = 1 - (1 - pct) * (1 - pct);
        x = (min * pct) + (max * (1 - pct));
        return x;
    }
};
