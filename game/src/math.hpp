#pragma once
#include "pch.hpp"

namespace tl {

struct Vec2 {
    union { float x, w; };
    union { float y, h; };

    static const Vec2 ZERO;
    static const Vec2 ONES;
    static const Vec2 X_AXIS;
    static const Vec2 Y_AXIS;

    Vec2(): x{0}, y{0} {}
    explicit Vec2(float value): x{value}, y{value} {}
    Vec2(float x, float y): x{x}, y{y} {}

    Vec2& operator+=(const Vec2& o);
    Vec2& operator-=(const Vec2& o);
    Vec2& operator*=(const Vec2& o);
    Vec2& operator/=(const Vec2& o);
    Vec2& operator*=(float value);
    Vec2& operator/=(float value);
    bool operator==(const Vec2& o) const;
    bool operator!=(const Vec2& o) const;

    float Dot(const Vec2&) const;
    float Cross(const Vec2&) const;
    Vec2 Normalize() const;
    void ToNormalize();
    float LengthSqrd() const;
    float Length() const;
};

Vec2 operator+(const Vec2& v1, const Vec2& v2);
Vec2 operator-(const Vec2& v1, const Vec2& v2);
Vec2 operator*(const Vec2& v1, const Vec2& v2);
Vec2 operator/(const Vec2& v1, const Vec2& v2);
Vec2 operator*(const Vec2& v1, float value);
Vec2 operator/(const Vec2& v1, float value);
Vec2 operator*(float value, const Vec2& v1);
Vec2 operator/(float value, const Vec2& v1);
Vec2 operator-(const Vec2& v);

struct Color {
    unsigned char r, g, b, a = 255;
};

struct Circle {
    float radius;
    Vec2 center;
};

struct Rect {
    Vec2 position, size;

    Rect() = default;
    Rect(const Vec2& pos, const Vec2& size): position{pos}, size{size} {}
    Rect(float x, float y, float w, float h): position{x, y}, size{w, h} {}

    static Rect CreateFromTopLeft(const Vec2& pos, const Vec2& size) {
        return {pos, size};
    }

    static Rect CreateFromCenter(const Vec2& center, const Vec2& halfSize) {
        return {center - halfSize, halfSize * 2.0f};
    }
};

struct Line {
    Vec2 p;
    Vec2 dir;

    Line(const Vec2& p, const Vec2& dir): p{p}, dir{dir} {}

    Vec2 GetPtOn(float t) const {
        return p + t * dir;
    }
};

struct Segment: public Line {
    float t1, t2;   // t1 <= t2

    Segment(const Vec2& p, const Vec2& dir, float t1, float t2): Line{p, dir}, t1{std::min(t1, t2)}, t2{std::max(t1, t2)} {}

    bool IsOn(float t) const {
        return t >= t1 && t <= t2;
    }
};

const float PI = 3.141592653589;

float Deg2Rad(float deg);
float Rad2Deg(float rad);

Vec2 Rotate(const Vec2& p, float degree);

}
