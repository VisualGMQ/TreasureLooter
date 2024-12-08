#include "transform.hpp"
#include "log.hpp"

namespace tl {

bool Transform::operator==(const Transform& o) const noexcept {
    return o.rotation == rotation && o.position == position && o.scale == scale;
}

bool Transform::operator!=(const Transform& o) const noexcept {
    return !(*this == o);
}

Transform CalcTransformFromParent(const Transform& parentGlobalTransform,
                                  const Transform& localTransform) {
    Transform trans;
    trans.scale = localTransform.scale * parentGlobalTransform.scale;
    trans.rotation = localTransform.rotation + parentGlobalTransform.rotation;
    trans.position =
        Rotate(localTransform.position * parentGlobalTransform.scale,
               parentGlobalTransform.rotation);
    trans.position += parentGlobalTransform.position;
    return trans;
}

Transform CalcLocalTransformToParent(const Transform& parentGlobalTransform,
                                     const Transform& globalTrans) {
    if (parentGlobalTransform.scale.x == 0 ||
        parentGlobalTransform.scale.y == 0) {
        LOGE("divide with zero");
    }

    Transform trans;
    trans.scale = globalTrans.scale / parentGlobalTransform.scale;
    trans.rotation = globalTrans.rotation - parentGlobalTransform.rotation;
    trans.position = (globalTrans.position - parentGlobalTransform.position) /
                     parentGlobalTransform.scale;
    trans.position = Rotate(trans.position, -parentGlobalTransform.rotation);

    return trans;
}

}  // namespace tl
