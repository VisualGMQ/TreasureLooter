#pragma once
#include "entity.hpp"
#include "log.hpp"

template <typename T>
class ComponentManager {
public:
    template <typename... Args>
    void RegisterEntity(Entity entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        m_components.emplace(entity,
                             std::make_unique<T>(std::forward<Args>(args)...));
    }

    void RemoveEntity(Entity entity) { m_components.erase(entity); }

    void ReplaceComponent(Entity entity, const T& component) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            *it->second = component;
            return;
        }

        m_components.emplace(entity, std::make_unique<T>(component));       
    }

    bool Has(Entity entity) const {
        return m_components.find(entity) != m_components.end();
    }

    const T* Get(Entity entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    T* Get(Entity entity) {
        return const_cast<T*>(std::as_const(*this).Get(entity));
    }

protected:
    std::unordered_map<Entity, std::unique_ptr<T>> m_components;
};