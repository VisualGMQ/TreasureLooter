#include "math.hpp"

namespace tl {

const Vec2 Vec2::ZERO{};
const Vec2 Vec2::ONES{1, 1};
const Vec2 Vec2::X_AXIS{1, 0};
const Vec2 Vec2::Y_AXIS{0, 1};

Vec2 operator+(const Vec2& v1, const Vec2& v2) {
    return {v1.x + v2.x, v1.y + v2.y};
}

Vec2 operator-(const Vec2& v1, const Vec2& v2) {
    return {v1.x - v2.x, v1.y - v2.y};
}

Vec2 operator*(const Vec2& v1, const Vec2& v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

Vec2 operator/(const Vec2& v1, const Vec2& v2) {
    return {v1.x / v2.x, v1.y / v2.y};
}

Vec2 operator-(const Vec2& v) {
    return {-v.x, -v.y};
}

Vec2 operator*(const Vec2& v1, float value) {
    return {v1.x * value, v1.y * value};
}

Vec2 operator/(const Vec2& v1, float value) {
    return {v1.x / value, v1.y / value};
}

Vec2 operator*(float value, const Vec2& v1) {
    return v1 * value;
}

Vec2 operator/(float value, const Vec2& v1) {
    return v1 / value;
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

Vec2& Vec2::operator*=(const Vec2& o) {
    x *= o.x;
    y *= o.y;
    return *this;
}

Vec2& Vec2::operator/=(const Vec2& o) {
    x /= o.x;
    y /= o.y;
    return *this;
}

Vec2& Vec2::operator*=(float value) {
    *this = *this * value;
    return *this;
}

Vec2& Vec2::operator/=(float value) {
    *this = *this / value;
    return *this;
}

bool Vec2::operator==(const Vec2& o) const {
    return x == o.x && y == o.y;
}

bool Vec2::operator!=(const Vec2& o) const {
    return !(*this == o);
}

float Vec2::Dot(const Vec2& o) const {
    return x * o.x + y * o.y; 
}

float Vec2::Cross(const Vec2& o) const {
    return x * o.y - y * o.x;
}

Vec2 Vec2::Normalize() const {
    float len = Length();
    return *this / len;
}

void Vec2::ToNormalize() {
    float len = Length();
    x /= len;
    y /= len;
}

float Vec2::LengthSqrd() const {
    return Dot(*this);
}

float Vec2::Length() const {
    return std::sqrt(LengthSqrd()); 
}

float Deg2Rad(float deg) {
    return deg * PI / 180.0;
}

float Rad2Deg(float rad) {
    return rad * 180.0 / PI;
}

Vec2 Rotate(const Vec2& p, float degree) {
    float rad = Deg2Rad(degree);
    float cos = std::cos(rad);
    float sin = std::sin(rad);

    return Vec2{cos * p.x - sin * p.y, sin * p.x + cos * p.y};
}

bool IsPointInCircle(const Vec2& p, const Circle& c) {
    return (p - c.center).LengthSqrd() <= c.radius * c.radius;
}

}
