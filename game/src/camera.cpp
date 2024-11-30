#include "camera.hpp"
#include "gameobject.hpp"

namespace tl {

Camera::operator bool() const noexcept {
    return enable;
}

const Vec2& Camera::GetGlobalOffset() const {
    return globalOffset_;
}

void Camera::Update(const GameObject& ownerGO) {
    globalOffset_ = offset + ownerGO.GetGlobalTransform().position;
}

}  // namespace tl
