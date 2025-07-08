#pragma once

struct Vec2 final {
    union {
        float w;
        float x;
    };

    union {
        float h;
        float y;
    };

    Vec2();
    Vec2(float x, float y);

    Vec2& operator*=(const Vec2&);
    Vec2& operator*=(float);
    Vec2& operator/=(const Vec2&);
    Vec2& operator/=(float);
    Vec2& operator+=(const Vec2&);
    Vec2& operator-=(const Vec2&);

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

    float Value() const { return m_value; }

private:
    float m_value{};
};

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

struct Pose {
    Vec2 m_position;
    Degrees m_rotation;
    float m_scale{1.0};

    Pose operator*(const Pose& o) const;
};
