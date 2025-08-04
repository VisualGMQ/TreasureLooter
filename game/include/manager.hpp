#pragma once
#include "entity.hpp"
#include "handle.hpp"
#include "log.hpp"

template <typename T>
class ComponentManager {
public:
    using component_type =
        std::conditional_t<is_handle_v<T>, T, std::unique_ptr<T>>;
    using expose_type = std::conditional_t<is_handle_v<T>, T, T*>;

    template <typename... Args>
    void RegisterEntity(Entity entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, std::forward<Args>(args)...);
        } else {
            m_components.emplace(
                entity, std::make_unique<T>(std::forward<Args>(args)...));
        }
    }

    template <typename U, typename... Args>
    void RegisterEntityByDerive(Entity entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, std::forward<Args>(args)...);
        } else {
            m_components.emplace(
                entity, std::make_unique<U>(std::forward<Args>(args)...));
        }
    }

    void RemoveEntity(Entity entity) { m_components.erase(entity); }

    void ReplaceComponent(Entity entity, const T& component) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            if constexpr (is_handle) {
                it->second = component;
            } else {
                *it->second = component;
            }
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, component);
        } else {
            m_components.emplace(entity, std::make_unique<T>(component));
        }
    }

    bool Has(Entity entity) const {
        return m_components.find(entity) != m_components.end();
    }

    const expose_type Get(Entity entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            if constexpr (is_handle) {
                return it->second;
            } else {
                return it->second.get();
            }
        }
        return nullptr;
    }

    expose_type Get(Entity entity) {
        if constexpr (is_handle) {
            return std::as_const(*this).Get(entity);
        } else {
            return const_cast<expose_type>(std::as_const(*this).Get(entity));
        }
    }

protected:
    static constexpr bool is_handle = is_handle_v<T>;

    std::unordered_map<Entity, component_type> m_components;
};