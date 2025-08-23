#pragma once
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <vector>

template <typename T>
struct TVec2 final {
    union {
        T w;
        T x;
    };

    union {
        T h;
        T y;
    };

    static TVec2 ZERO;
    static TVec2 X_UNIT;
    static TVec2 Y_UNIT;

    TVec2();

    template <typename U>
    explicit TVec2(const TVec2<U> &o) {
        x = static_cast<T>(o.x);
        y = static_cast<T>(o.y);
    }

    TVec2(T x, T y);

    TVec2 &operator*=(const TVec2 &);

    TVec2 &operator*=(float);

    TVec2 &operator/=(const TVec2 &);

    TVec2 &operator/=(float);

    TVec2 &operator+=(const TVec2 &);

    TVec2 &operator-=(const TVec2 &);

    bool operator==(const TVec2 &o) const { return x == o.x && y == o.y; }

    bool operator!=(const TVec2 &o) const { return !(*this == o); }

    float Dot(const TVec2 &) const;

    float Cross(const TVec2 &) const;

    float LengthSquared() const;

    float Length() const;

    TVec2 Normalize() const;
};

template <typename T>
TVec2<T> TVec2<T>::ZERO{};

template <typename T>
TVec2<T> TVec2<T>::X_UNIT{1, 0};

template <typename T>
TVec2<T> TVec2<T>::Y_UNIT{0, 1};

template <typename T>
TVec2<T> operator*(float, const TVec2<T> &);

template <typename T>
TVec2<T> operator*(const TVec2<T> &, float);

template <typename T>
TVec2<T> operator*(const TVec2<T> &, const TVec2<T> &);

template <typename T>
TVec2<T> operator/(const TVec2<T> &, float);

template <typename T>
TVec2<T> operator/(const TVec2<T> &, const TVec2<T> &);

template <typename T>
TVec2<T> operator+(const TVec2<T> &, const TVec2<T> &);

template <typename T>
TVec2<T> operator-(const TVec2<T> &, const TVec2<T> &);

template <typename T>
float Dot(const TVec2<T> &, const TVec2<T> &);

template <typename T>
float Cross(const TVec2<T> &, const TVec2<T> &);

template <typename T>
TVec2<T> operator-(const TVec2<T> &);

template <typename T>
std::ostream &operator<<(std::ostream &os, const TVec2<T> &);

using Vec2 = TVec2<float>;
using Vec2I = TVec2<int>;
using Vec2UI = TVec2<uint32_t>;

template <typename T>
TVec2<T>::TVec2() : x{0}, y{0} {}

template <typename T>
TVec2<T>::TVec2(T x, T y) : x{x}, y{y} {}

template <typename T>
TVec2<T> &TVec2<T>::operator*=(const TVec2 &o) {
    x *= o.x;
    y *= o.y;
    return *this;
}

template <typename T>
TVec2<T> &TVec2<T>::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

template <typename T>
TVec2<T> &TVec2<T>::operator/=(const TVec2 &o) {
    x /= o.x;
    y /= o.y;
    return *this;
}

template <typename T>
TVec2<T> &TVec2<T>::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

template <typename T>
TVec2<T> &TVec2<T>::operator+=(const TVec2 &o) {
    x += o.x;
    y += o.y;
    return *this;
}

template <typename T>
TVec2<T> &TVec2<T>::operator-=(const TVec2 &o) {
    x -= o.x;
    y -= o.y;
    return *this;
}

template <typename T>
float TVec2<T>::Dot(const TVec2 &o) const {
    return x * o.x + y * o.y;
}

template <typename T>
float TVec2<T>::Cross(const TVec2 &o) const {
    return x * o.y - y * o.x;
}

template <typename T>
float TVec2<T>::LengthSquared() const {
    return x * x + y * y;
}

template <typename T>
float TVec2<T>::Length() const {
    return std::sqrt(LengthSquared());
}

template <typename T>
TVec2<T> TVec2<T>::Normalize() const {
    float len = Length();
    if (len <= std::numeric_limits<float>::epsilon()) {
        return {};
    }
    return *this / len;
}

template <typename T>
TVec2<T> operator*(float x, const TVec2<T> &v) {
    TVec2<T> o = v;
    return o *= x;
}

template <typename T>
TVec2<T> operator*(const TVec2<T> &v, float x) {
    TVec2<T> o = v;
    return o *= x;
}

template <typename T>
TVec2<T> operator*(const TVec2<T> &v1, const TVec2<T> &v2) {
    TVec2<T> v3 = v1;
    return v3 *= v2;
}

template <typename T>
TVec2<T> operator/(const TVec2<T> &v, float x) {
    TVec2<T> o = v;
    return o /= x;
}

template <typename T>
TVec2<T> operator/(const TVec2<T> &v1, const TVec2<T> &v2) {
    TVec2<T> v3 = v1;
    return v3 /= v2;
}

template <typename T>
TVec2<T> operator+(const TVec2<T> &v1, const TVec2<T> &v2) {
    TVec2<T> v3 = v1;
    return v3 += v2;
}

template <typename T>
TVec2<T> operator-(const TVec2<T> &v1, const TVec2<T> &v2) {
    TVec2<T> v3 = v1;
    return v3 -= v2;
}

template <typename T>
float Dot(const TVec2<T> &v1, const TVec2<T> &v2) {
    return v1.Dot(v2);
}

template <typename T>
float Cross(const TVec2<T> &v1, const TVec2<T> &v2) {
    return v1.Cross(v2);
}

template <typename T>
TVec2<T> operator-(const TVec2<T> &o) {
    return TVec2<T>(-o.x, -o.y);
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const TVec2<T> &p) {
    os << "TVec2<T>(" << p.x << ", " << p.y << ")";
    return os;
}

Vec2 Reflect(const Vec2& v, const Vec2& n);

struct Color {
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Purple;
    static const Color Black;
    static const Color White;

    float r{}, g{}, b{}, a = 1;
};

struct Radians;

struct Degrees {
    Degrees() = default;

    Degrees(float value);

    Degrees(Radians);

    Degrees &operator=(Radians);
    Degrees &operator=(float);
    Degrees &operator-=(Degrees);
    Degrees &operator+=(Degrees);
    Degrees &operator*=(Degrees);
    Degrees &operator/=(Degrees);

    bool operator>(Degrees) const;
    bool operator<(Degrees) const;
    bool operator>=(Degrees) const;
    bool operator<=(Degrees) const;
    bool operator==(Degrees) const;
    bool operator!=(Degrees) const;

    float Value() const { return m_value; }

private:
    float m_value{};
};

Degrees operator+(Degrees d1, Degrees d2);
Degrees operator-(Degrees d1, Degrees d2);
Degrees operator*(Degrees d1, Degrees d2);
Degrees operator/(Degrees d1, Degrees d2);

struct Radians {
    Radians() = default;

    Radians(float value);

    Radians(Degrees);

    Radians &operator=(Degrees);
    Radians &operator=(float);
    Radians &operator-=(Radians);
    Radians &operator+=(Radians);
    Radians &operator*=(Radians);
    Radians &operator/=(Radians);

    bool operator>(Radians) const;
    bool operator<(Radians) const;
    bool operator>=(Radians) const;
    bool operator<=(Radians) const;
    bool operator==(Radians) const;
    bool operator!=(Radians) const;

    float Value() const { return m_value; }

private:
    float m_value{};
};

Radians operator+(Radians d1, Radians d2);
Radians operator-(Radians d1, Radians d2);
Radians operator*(Radians d1, Radians d2);
Radians operator/(Radians d1, Radians d2);

inline const Radians PI{3.14159265358979323846f};
inline const Radians PI_Half{3.14159265358979323846f * 0.5};


struct Mat33 {
    static Mat33 CreateTranslation(const Vec2 &);

    static Mat33 CreateScale(const Vec2 &);

    static Mat33 CreateRotation(Degrees);

    Mat33();

    float Get(size_t x, size_t y) const;

    float &Get(size_t x, size_t y);

    void Set(size_t x, size_t y, float value);

    Mat33 &operator*=(const Mat33 &);

private:
    float m_data[3][3] = {0};
};

Mat33 operator*(const Mat33 &, const Mat33 &);

template <typename T>
class MatStorage {
public:
    MatStorage() = default;

    MatStorage(size_t w, size_t h) { Resize(w, h); }

    bool InRange(int x, int y) const { return x < m_w && y < m_h; }

    void Resize(size_t w, size_t h) {
        m_w = w;
        m_h = h;

        size_t old_size = m_data.size();
        m_data.resize(w);
        if (old_size < m_data.size()) {
            for (size_t i = old_size; i < m_data.size(); i++) {
                m_data[i].resize(h);
            }
        }
    }

    void ExpandTo(size_t w, size_t h) {
        if (w > m_w) {
            m_w = w;
            m_data.resize(w);
        }

        if (h > m_h) {
            m_h = h;
        }
        for (size_t i = 0; i < m_data.size(); i++) {
            if (m_data[i].size() < m_h) {
                m_data[i].resize(m_h);
            }
        }
    }

    const T &Get(size_t x, size_t y) const { return m_data[x][y]; }

    T &Get(size_t x, size_t y) { return m_data[x][y]; }

    void Set(const T &value, size_t x, size_t y) { m_data[x][y] = value; }

    void Set(T &&value, size_t x, size_t y) { m_data[x][y] = std::move(value); }

    void Clear() {
        m_w = 0;
        m_h = 0;
        m_data.clear();
    }

    size_t GetWidth() const { return m_w; }

    size_t GetHeight() const { return m_h; }

    size_t GetSize() const { return GetWidth() * GetHeight(); }

private:
    std::vector<std::vector<T> > m_data;
    size_t m_w{}, m_h{};
};

Vec2 Rotate(const Vec2 &, Degrees);

struct Region {
    Vec2 m_topleft, m_size;
};

/**
 * present a range, value in [m_a, m_b)
 */
template <typename T>
struct Range {
    T m_begin{}, m_end{};
};

template <typename T>
struct Range2D {
    Range<T> m_x;
    Range<T> m_y;
};

struct Transform {
    friend class RelationshipManager;

    Vec2 m_position;
    Degrees m_rotation;
    Vec2 m_scale{1.0, 1.0};

    const Mat33 &GetLocalMat() const;

    const Mat33 &GetGlobalMat() const;

    void UpdateMat(const Transform *parent);

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

/**
 *
 * @param norm_a normalized vector
 * @param norm_b normalized vector
 * @return radians of angle between norm_a & norm_b in [0, 2PI]
 */
Radians GetAngle(const Vec2 &norm_a, const Vec2 &norm_b);

struct DecompositionResult {
    Vec2 m_tangent;
    Vec2 m_normal;
};

/**
 * @param v  the vector be decomposed
 * @param normal normalized vector
 */
DecompositionResult DecomposeVector(const Vec2 &v, const Vec2 &normal);

// for spdlog output
template <>
struct fmt::formatter<Vec2> : fmt::ostream_formatter {};
