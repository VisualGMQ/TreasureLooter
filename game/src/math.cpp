#include "math.hpp"

#include "../3rdlibs/SDL/src/video/khronos/vulkan/vulkan_core.h"

#include <valarray>

Vec2::Vec2() : x{0}, y{0} {}

Vec2::Vec2(float x, float y) : x{x}, y{y} {}

Vec2& Vec2::operator*=(const Vec2& o) {
    x *= o.x;
    y *= o.y;
    return *this;
}

Vec2& Vec2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vec2& Vec2::operator/=(const Vec2& o) {
    x /= o.x;
    y /= o.y;
    return *this;
}

Vec2& Vec2::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

Vec2& Vec2::operator+=(const Vec2& o) {
    x += o.x;
    y += o.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& o) {
    x -= o.x;
    y -= o.y;
    return *this;
}

float Vec2::Dot(const Vec2& o) const {
    return x * o.x + y * o.y;
}

float Vec2::Cross(const Vec2& o) const {
    return x * o.y - y * o.x;
}

float Vec2::LengthSquared() const {
    return x * x + y * y;
}

float Vec2::Length() const {
    return std::sqrt(LengthSquared());
}

Vec2 operator*(float x, const Vec2& v) {
    Vec2 o = v;
    return o *= x;
}

Vec2 operator*(const Vec2& v, float x) {
    Vec2 o = v;
    return o *= x;
}

Vec2 operator*(const Vec2& v1, const Vec2& v2) {
    Vec2 v3 = v1;
    return v3 *= v2;
}

Vec2 operator/(const Vec2& v, float x) {
    Vec2 o = v;
    return o /= x;
}

Vec2 operator/(const Vec2& v1, const Vec2& v2) {
    Vec2 v3 = v1;
    return v3 /= v2;
}

Vec2 operator+(const Vec2& v1, const Vec2& v2) {
    Vec2 v3 = v1;
    return v3 += v2;
}

Vec2 operator-(const Vec2& v1, const Vec2& v2) {
    Vec2 v3 = v1;
    return v3 -= v2;
}

float Dot(const Vec2& v1, const Vec2& v2) {
    return v1.Dot(v2);
}

float Cross(const Vec2& v1, const Vec2& v2) {
    return v1.Cross(v2);
}

Vec2 operator-(const Vec2& o) {
    return Vec2(-o.x, -o.y);
}

Degrees::Degrees(float value) : m_value{value} {}

Degrees::Degrees(Radians radians) {
    *this = radians;
}

Degrees& Degrees::operator=(Radians radians) {
    m_value = radians.Value() * 180.0f / PI;
    return *this;
}

Degrees& Degrees::operator=(float value) {
    m_value = value;
    return *this;
}

Radians::Radians(float value) : m_value{value} {}

Radians::Radians(Degrees degrees) {
    *this = degrees;
}

Radians& Radians::operator=(Degrees degrees) {
    m_value = degrees.Value() * PI / 180.0f;
    return *this;
}

Radians& Radians::operator=(float value) {
    m_value = value;
    return *this;
}

Vec2 Rotate(const Vec2& p, Degrees d) {
    float rot = d.Value();
    auto s = std::sin(rot);
    auto c = std::cos(rot);

    return {p.x * c - p.y * s, p.x * s + p.y * c};
}

Pose Pose::operator*(const Pose& o) const {
    Pose pose;
    pose.m_rotation = m_rotation.Value() + o.m_rotation.Value();
    pose.m_scale = o.m_scale * m_scale;
    pose.m_position = Rotate(o.m_position * pose.m_scale, pose.m_rotation) + m_position;
    return pose;
}
