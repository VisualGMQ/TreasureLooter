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

const Color Color::White{1, 1, 1, 1};
const Color Color::Black{0, 0, 0, 1};
const Color Color::Red{1, 0, 0, 1};
const Color Color::Blue{0, 1, 0, 1};
const Color Color::Green{0, 0, 1, 1};
const Color Color::Yellow{1, 1, 0, 1};

Color& Color::operator*=(const Color& o) {
    r *= o.r;
    g *= o.g;
    b *= o.b;
    a *= o.a;
    return *this;
}

Color& Color::operator*=(float value) {
    r *= value;
    g *= value;
    b *= value;
    a *= value;
    return *this;
}

Color& Color::operator/=(float value) {
    r /= value;
    g /= value;
    b /= value;
    a /= value;
    return *this;
}

bool Color::operator==(const Color& o) const {
    return r == o.r && g == o.g && b == o.b && a == o.a;
}

bool Color::operator!=(const Color& o) const {
    return !(*this == o);
}

Color operator*(const Color& c1, const Color& c2) {
    Color c = c1;
    c.r *= c2.r;
    c.g *= c2.g;
    c.b *= c2.b;
    c.a *= c2.a;
    return c;
}

Color operator/(const Color& c1, const Color& c2) {
    Color c = c1;
    c.r /= c2.r;
    c.g /= c2.g;
    c.b /= c2.b;
    c.a /= c2.a;
    return c;
}

Color operator*(const Color& c1, float value) {
    Color c = c1;
    c.r *= value;
    c.g *= value;
    c.b *= value;
    c.a *= value;
    return c;
}

Color operator/(const Color& c1, float value) {
    Color c = c1;
    c.r /= value;
    c.g /= value;
    c.b /= value;
    c.a /= value;
    return c;
}

Color operator*(float value, const Color& c1) {
    return c1 * value;
}

Color operator/(float value, const Color& c1) {
    return c1 / value;
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

Vec2 ProjectOn(const Vec2& v, const Vec2& dir) {
    return v.Dot(dir) * dir;
}

}  // namespace tl
