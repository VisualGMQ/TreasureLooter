#pragma once

namespace tl {

template <typename T, typename Friend>
class ID {
public:
    friend Friend;

    using underlying_type = uint32_t;

    ID(const ID&) = default;
    ID() : id_{Invalid} {}

    operator bool() const { return id_ != Invalid; }

    ID& operator=(const ID&) = default;

    bool operator==(const ID& o) const {
        return id_ == o.id_;
    }

    bool operator!=(const ID& o) const {
        return !(*this == o);
    }

    explicit operator underlying_type() const {
        return id_;
    }

private:
    underlying_type id_;

    static const underlying_type Invalid = 0;

    ID(underlying_type id) : id_{id} {}
};

}