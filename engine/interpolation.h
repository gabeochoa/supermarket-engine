
#pragma once

#include <cmath>

struct BaseInterp {
    float min;
    float max;
    int steps;
    int step;
    float x;

    BaseInterp() : min(0.f), max(0.f), steps(10) {
        x = min;
        step = 0;
    }

    BaseInterp(float mn, float mx, int s) : min(mn), max(mx), steps(s) {
        x = min;
        step = 0;
    }

    virtual float next() = 0;
};

struct LinearInterp : public BaseInterp {
    LinearInterp() : BaseInterp() {}
    LinearInterp(float mn, float mx, int s) : BaseInterp(mn, mx, s) {}

    virtual float next() override {
        float pct = fmin(1.f, 1.f * step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

#define M_SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
struct SmoothstepInterp : public BaseInterp {
    virtual float next() override {
        float pct = M_SMOOTHSTEP(step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

#define M_SMOOTHERSTEP(x) ((x) * (x) * (x) * ((x) * ((x)*6 - 15) + 10))
struct SmootherstepInterp : public BaseInterp {
    virtual float next() override {
        float pct = M_SMOOTHERSTEP(step / steps);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

struct SlowAccelInterp : public BaseInterp {
    virtual float next() override {
        float pct = (step / steps);
        pct = pct * pct;
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};

struct SlowDecellInterp : public BaseInterp {
    virtual float next() override {
        float pct = (step / steps);
        pct = 1 - (1 - pct) * (1 - pct);
        x = (max * pct) + (min * (1 - pct));
        step++;
        return x;
    }
};
