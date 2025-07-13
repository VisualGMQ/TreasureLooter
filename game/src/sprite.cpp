#include "sprite.hpp"

#include "context.hpp"
#include "transform.hpp"

void SpriteManager::Update() {
    auto& renderer = Context::GetInst().m_renderer;
    auto& transform_manager = Context::GetInst().m_transform_manager;
    for (auto& [entity, sprite] : m_components) {
        const Transform* transform = transform_manager->Get(entity);
        if (!transform) {
            continue;
        }
        auto src_region = sprite->m_region;
        Region dst_region;
        auto& global_pose = transform->m_global_pose;
        auto image_size = sprite->m_region.m_size;
        dst_region.m_topleft = global_pose.m_position - image_size * 0.5;
        dst_region.m_size = sprite->m_size * global_pose.m_scale;

        renderer->DrawImage(*sprite->m_image, src_region, dst_region,
                            global_pose.m_rotation.Value(),
                            dst_region.m_size * 0.5, sprite->m_flip);
    }
}