#include "transform.hpp"

namespace tl {

Transform CalcTransformFromParent(const Transform& parentGlobalTransform,
                                  const Transform& localTransform) {
    Transform trans;
    trans.scale = localTransform.scale * parentGlobalTransform.scale;
    trans.rotation = localTransform.rotation + parentGlobalTransform.rotation;
    float radians = Deg2Rad(localTransform.rotation);
    float cos = std::cos(radians);
    float sin = std::sin(radians);
    trans.position =
        Rotate(localTransform.position * parentGlobalTransform.scale,
               parentGlobalTransform.rotation);
    trans.position += parentGlobalTransform.position;
    return trans;
}

Transform CalcLocalTransformToParent(const Transform& parentGlobalTransform,
                                  const Transform& globalTrans) {
    Transform trans;
    trans.scale = globalTrans.scale / parentGlobalTransform.scale;
    trans.rotation = globalTrans.rotation - parentGlobalTransform.rotation;
    trans.position = (globalTrans.position - parentGlobalTransform.position) / parentGlobalTransform.scale;
    trans.position = Rotate(trans.position, -parentGlobalTransform.rotation);

    return trans;
}

}  // namespace tl
