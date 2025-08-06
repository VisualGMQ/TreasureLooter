#pragma once
#include <cstddef>
#include <vector>

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

    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }

    bool operator!=(const Vec2& o) const { return !(*this == o); }

    float Dot(const Vec2&) const;
    float Cross(const Vec2&) const;
    float LengthSquared() const;
    float Length() const;
    Vec2 Normalize() const;
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
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Purple;

    float r{}, g{}, b{}, a = 1;
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

template <typename T>
class MatStorage {
public:
    MatStorage() = default;

    MatStorage(size_t w, size_t h) : m_w{w}, m_h{h} {
        m_data.resize(m_w * m_h);
    }

    bool InRange(int x, int y) const { return x < m_w && y < m_h; }

    void Resize(size_t w, size_t h) {
        m_w = w;
        m_h = h;
        m_data.resize(w * h);
    }

    const T& Get(size_t x, size_t y) const { return m_data[x + y * m_w]; }

    T& Get(size_t x, size_t y) { return m_data[x + y * m_w]; }

    void Set(const T& value, size_t x, size_t y) {
        m_data[x + y * m_w] = value;
    }

    void Set(T&& value, size_t x, size_t y) {
        m_data[x + y * m_w] = std::move(value);
    }

    void Clear() {
        m_w = 0;
        m_h = 0;
        m_data.clear();
    }

    size_t GetWidth() const { return m_w; }

    size_t GetHeight() const { return m_h; }

private:
    std::vector<T> m_data;
    size_t m_w{}, m_h{};
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

template <typename T>
T Clamp(T v, T a, T b) {
    return v < a ? a : v > b ? b : v;
}

struct DecompositionResult {
    Vec2 m_tangent;
    Vec2 m_normal;
};

/**
 * @param v  the vector be decomposed
 * @param normal normalized vector
 */
DecompositionResult DecomposeVector(const Vec2& v, const Vec2& normal);