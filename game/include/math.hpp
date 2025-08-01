#pragma once
#include <cstddef>

struct Vec2 final {
    union {
        float w;
        float x;
    };

    union {
        float h;
        float y;
    };

    static Vec2 ZERO;
    static Vec2 X_UNIT;
    static Vec2 Y_UNIT;

    Vec2();
    Vec2(float x, float y);

    Vec2& operator*=(const Vec2&);
    Vec2& operator*=(float);
    Vec2& operator/=(const Vec2&);
    Vec2& operator/=(float);
    Vec2& operator+=(const Vec2&);
    Vec2& operator-=(const Vec2&);

    bool operator==(const Vec2& o) const {
        return x == o.x && y == o.y;
    }

    bool operator!=(const Vec2& o) const {
        return !(*this == o);
    }

    float Dot(const Vec2&) const;
    float Cross(const Vec2&) const;
    float LengthSquared() const;
    float Length() const;
};

Vec2 operator*(float, const Vec2&);
Vec2 operator*(const Vec2&, float);
Vec2 operator*(const Vec2&, const Vec2&);
Vec2 operator/(const Vec2&, float);
Vec2 operator/(const Vec2&, const Vec2&);
Vec2 operator+(const Vec2&, const Vec2&);
Vec2 operator-(const Vec2&, const Vec2&);
float Dot(const Vec2&, const Vec2&);
float Cross(const Vec2&, const Vec2&);
Vec2 operator-(const Vec2&);

struct Color {
    float r{}, g{}, b{}, a = 1;
};

struct Rect {
    Vec2 m_center;
    Vec2 m_half_size;
};

struct Circle {
    float m_radius = 0;
    Vec2 m_position;
};

constexpr float PI = 3.14159265358979323846f;

struct Radians;

struct Degrees {
    Degrees() = default;
    Degrees(float value);
    Degrees(Radians);
    Degrees& operator=(Radians);
    Degrees& operator=(float);

    Degrees& operator-=(Degrees);
    Degrees& operator+=(Degrees);
    Degrees& operator*=(Degrees);
    Degrees& operator/=(Degrees);

    float Value() const { return m_value; }

private:
    float m_value{};
};

Degrees operator+(Degrees d1, Degrees d2);
Degrees operator-(Degrees d1, Degrees d2);
Degrees operator*(Degrees d1, Degrees d2);
Degrees operator/(Degrees d1, Degrees d2);

struct Mat33 {
    static Mat33 CreateTranslation(const Vec2&);
    static Mat33 CreateScale(const Vec2&);
    static Mat33 CreateRotation(Degrees);

    Mat33();
    float Get(size_t x, size_t y) const;
    float& Get(size_t x, size_t y);
    void Set(size_t x, size_t y, float value);

    Mat33& operator*=(const Mat33&);

private:
    float m_data[3][3] = {0};
};

Mat33 operator*(const Mat33&, const Mat33&);

struct Radians {
    Radians() = default;
    Radians(float value);
    Radians(Degrees);
    Radians& operator=(Degrees);
    Radians& operator=(float);

    float Value() const { return m_value; }

private:
    float m_value{};
};

Vec2 Rotate(const Vec2&, Degrees);

struct Region {
    Vec2 m_topleft, m_size;
};

struct Transform {
    friend class RelationshipManager;
    
    Vec2 m_position;
    Degrees m_rotation;
    Vec2 m_scale{1.0, 1.0};

    const Mat33& GetLocalMat() const;
    const Mat33& GetGlobalMat() const;
    void UpdateMat(const Transform* parent);

private:
    Mat33 m_mat;
    Mat33 m_global_mat;
};

template <typename T>
T Lerp(T a, T b, float t) {
    return a + (b - a) * t;
}