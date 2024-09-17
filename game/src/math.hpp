#include "pch.hpp"

namespace tl {

struct BaseVec2 {
    float value1, value2;
};

struct Vec2: public BaseVec2 {
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

struct Circle {
    float radius;
    Vec2 center;
};

struct Rect {
    Vec2 position, size;

    Rect(const Vec2& pos, const Vec2& size): position{pos}, size{size} {}

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

}
