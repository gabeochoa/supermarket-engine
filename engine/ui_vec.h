
#pragma once 
#include <initializer_list>

namespace IUI {

template <typename T>
struct vec2t {
    T x, y;

    vec2t& operator-(vec2t<T> const& b) {
        this->x -= b.x;
        this->y -= b.y;
        return *this;
    }

    vec2t& operator-=(vec2t<T> const& b) {
        this->x -= b.x;
        this->y -= b.y;
        return *this;
    }

    vec2t& operator+(vec2t<T> const& b) {
        this->x += b.x;
        this->y += b.y;
        return *this;
    }

    vec2t& operator+=(vec2t<T> const& b) {
        this->x += b.x;
        this->y += b.y;
        return *this;
    }

    vec2t& operator*(T const& b) {
        this->x *= b;
        this->y *= b;
        return *this;
    }

    vec2t& operator*=(T const& b) {
        this->x *= b;
        this->y *= b;
        return *this;
    }

    vec2t& operator/=(T const& b) {
        this->x /= b;
        this->y /= b;
        return *this;
    }
};

template <typename T>
struct vec4t {
    union {
        struct {
            T x, y, z, w;
        };
        struct {
            T r, g, b, a;
        };
        struct {
            T s, t, p, q;
        };
    };

    vec4t(T val) : x(val), y(val), z(val), w(val) {}
    vec4t(std::initializer_list<T> const& items) {
        auto it = items.begin();
        if (it != items.end()) x = *(it)++;
        if (it != items.end()) y = *(it)++;
        if (it != items.end()) z = *(it)++;
        if (it != items.end()) w = *(it)++;
    }
    vec4t(T a = T(), T b = T(), T c = T(), T d = T())
        : x(a), y(b), z(c), w(d) {}
    vec4t(vec2t<T> a = vec2t<T>(), vec2t<T> b = vec2t<T>())
        : x(a.x), y(a.y), z(b.x), w(b.y) {}
};

typedef vec4t<float> vec4;
typedef vec2t<float> vec2;

}
