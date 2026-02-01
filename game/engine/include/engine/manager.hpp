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

    ComponentManager() = default;
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;


    template <typename... Args>
    void RegisterEntity(Entity entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, Component{component_type{std::forward<Args>(args)...}, true});
        } else {
            m_components.emplace(
                entity, Component{std::make_unique<T>(std::forward<Args>(args)...), true});
        }
    }

    template <typename U, typename... Args>
    void RegisterEntityByDerive(Entity entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, Component{component_type{std::forward<Args>(args)...}, true});
        } else {
            m_components.emplace(
                entity, Component{std::make_unique<U>(std::forward<Args>(args)...), true});
        }
    }

    void RemoveEntity(Entity entity) { m_components.erase(entity); }

    void ReplaceComponent(Entity entity, const T& component) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            if constexpr (is_handle) {
                it->second.m_component = component;
                it->second.m_enable = true;
            } else {
                *it->second.m_component = component;
                it->second.m_enable = true;
            }
            return;
        }

        if constexpr (is_handle) {
            m_components.emplace(entity, Component{component, true});
        } else {
            m_components.emplace(
                entity, Component{std::make_unique<T>(component), true});
        }
    }

    bool Has(Entity entity) const {
        return m_components.find(entity) != m_components.end();
    }

    bool IsEnable(Entity entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            return it->second.m_enable;
        }
        return false;
    }
    
    void Enable(Entity entity) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            it->second.m_enable = true;
        }
    }
 
    void Disable(Entity entity) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            it->second.m_enable = false;
        }
    }

    const expose_type Get(Entity entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            if (!it->second.m_enable) {
                return nullptr;
            }
            if constexpr (is_handle) {
                return it->second.m_component;
            } else {
                return it->second.m_component.get();
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

    struct Component {
        component_type m_component;
        bool m_enable = true;
    };
    std::unordered_map<Entity, Component> m_components;
};
