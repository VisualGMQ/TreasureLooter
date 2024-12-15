#pragma once

namespace tl {

template <typename T, typename Friend>
class ID {
public:
    friend Friend;

    using UnderlyingType = uint32_t;

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

    explicit operator UnderlyingType() const {
        return id_;
    }

private:
    UnderlyingType id_;

    static const UnderlyingType Invalid = 0;

    ID(UnderlyingType id) : id_{id} {}
};

}

template <typename T, typename Friend>
class std::hash<tl::ID<T, Friend>> {
public: 
    size_t operator()(const tl::ID<T, Friend>& id) const {
        using UnderlyingType = typename tl::ID<T, Friend>::UnderlyingType;
        return std::hash<UnderlyingType>{}(static_cast<UnderlyingType>(id));
    } 
};