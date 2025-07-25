#include "math.hpp"

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

Degrees& Degrees::operator-=(Degrees o) {
    m_value -= o.m_value;
    return *this;
}

Degrees& Degrees::operator+=(Degrees o) {
    m_value += o.m_value;
    return *this;
}

Degrees& Degrees::operator*=(Degrees o) {
    m_value *= o.m_value;
    return *this;
}

Degrees& Degrees::operator/=(Degrees o) {
    m_value /= o.m_value;
    return *this;
}

Degrees operator+(Degrees d1, Degrees d2) {
    return d1 += d2;
}

Degrees operator-(Degrees d1, Degrees d2) {
    return d1 -= d2;
}

Degrees operator*(Degrees d1, Degrees d2) {
    return d1 *= d2;
}

Degrees operator/(Degrees d1, Degrees d2) {
    return d1 /= d2;
}

Mat33 Mat33::CreateTranslation(const Vec2& p) {
    Mat33 result;
    result.Set(2, 0, p.x);
    result.Set(2, 1, p.y);
    return result;
}

Mat33 Mat33::CreateScale(const Vec2& s) {
    Mat33 result;
    result.Set(0, 0, s.x);
    result.Set(1, 1, s.y);
    return result;
}

Mat33 Mat33::CreateRotation(Degrees d) {
    Mat33 result;
    float r = Radians{d}.Value();
    float s = std::sin(r);
    float c = std::cos(r);
    result.Set(0, 0, c);
    result.Set(0, 1, s);
    result.Set(1, 0, -s);
    result.Set(1, 1, c);
    return result;
}

Mat33::Mat33() {
    m_data[0][0] = 1;
    m_data[1][1] = 1;
    m_data[2][2] = 1;
}

Mat33& Mat33::operator*=(const Mat33& o) {
    Mat33 result;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < 3; ++k) {
                sum += Get(k, i) * o.Get(j, k);
            }
            result.Set(j, i, sum);
        }
    }

    *this = result;
    return *this;
}

Mat33 operator*(const Mat33& m1, const Mat33& m2) {
    Mat33 m(m1);
    return m *= m2;
}

float Mat33::Get(size_t x, size_t y) const {
    return m_data[x][y];
}

float& Mat33::Get(size_t x, size_t y) {
    return m_data[x][y];
}

void Mat33::Set(size_t x, size_t y, float value) {
    m_data[x][y] = value;
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
    float rot = Radians{d}.Value();
    auto s = std::sin(rot);
    auto c = std::cos(rot);

    return {p.x * c - p.y * s, p.x * s + p.y * c};
}

const Mat33& Transform::GetLocalMat() const {
    return m_mat;
}

const Mat33& Transform::GetGlobalMat() const {
    return m_global_mat;
}

void Transform::UpdateMat(const Transform* parent) {
    m_mat = Mat33::CreateTranslation(m_position) *
            Mat33::CreateRotation(m_rotation) * Mat33::CreateScale(m_scale);
    if (parent) {
        m_global_mat = parent->GetGlobalMat() * m_mat;
    } else {
        m_global_mat = m_mat;
    }
}
