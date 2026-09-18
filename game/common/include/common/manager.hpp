#pragma once
#include "common/entity.hpp"
#include "common/handle.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"

#include <algorithm>

template <typename T, typename EntityT = LogicEntity>
class ComponentManager {
public:
    using component_type = std::unique_ptr<T>;
    using expose_type = T*;
    using entity_type = EntityT;

    ComponentManager() = default;
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;
    virtual ~ComponentManager() = default;

    /**
     * register component on entity
     */
    template <typename... Args>
    void RegisterEntity(EntityT entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        m_components.emplace(
            entity,
            Component{std::make_unique<T>(std::forward<Args>(args)...), true});
    }

    /**
     * register inherit type of component on entity
     */
    template <typename U, typename... Args>
    void RegisterEntityByDerive(EntityT entity, Args&&... args) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            LOGW("[Component]: entity {} already registered", entity);
            return;
        }

        m_components.emplace(
            entity,
            Component{std::make_unique<U>(std::forward<Args>(args)...), true});
    }

    void RemoveEntity(EntityT entity) { m_components.erase(entity); }

    [[nodiscard]] bool Has(EntityT entity) const {
        return m_components.find(entity) != m_components.end();
    }

    [[nodiscard]] bool IsEnable(EntityT entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            return it->second.m_enable;
        }
        return false;
    }

    virtual void Enable(EntityT entity) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            it->second.m_enable = true;
        }
    }

    virtual void Disable(EntityT entity) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            it->second.m_enable = false;
        }
    }

    [[nodiscard]] expose_type Get(EntityT entity) const {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            return it->second.m_component.get();
        }
        return nullptr;
    }

    [[nodiscard]] virtual expose_type Get(EntityT entity) {
        return const_cast<expose_type>(std::as_const(*this).Get(entity));
    }

    void Clear() { m_components.clear(); }

protected:
    struct Component {
        component_type m_component;
        bool m_enable = true;
    };

    std::unordered_map<EntityT, Component> m_components;

    template <typename U>
    void doReplaceComponent(EntityT entity, U&& component) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            *it->second.m_component = std::forward<U>(component);
            it->second.m_enable = true;
            return;
        }

        m_components.emplace(
            entity,
            Component{std::make_unique<T>(std::forward<U>(component)), true});
    }
};

template <typename T, typename EntityT = LogicEntity>
class MultiComponentManager {
public:
    using component_type = std::unique_ptr<T>;
    using expose_type = T*;
    using entity_type = EntityT;

    MultiComponentManager() = default;
    MultiComponentManager(const MultiComponentManager&) = delete;
    MultiComponentManager& operator=(const MultiComponentManager&) = delete;
    virtual ~MultiComponentManager() = default;

    template <typename... Args>
    expose_type AddComponent(EntityT entity, Args&&... args) {
        auto [it, _] =
            m_components.try_emplace(entity, std::vector<Component>{});
        it->second.push_back(
            Component{std::make_unique<T>(std::forward<Args>(args)...), true});
        return it->second.back().m_component.get();
    }

    void RemoveEntity(EntityT entity) { m_components.erase(entity); }

    void RemoveComponent(EntityT entity, T* component) {
        TL_RETURN_IF_NULL(component);

        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return;
        }

        auto& components = it->second;
        components.erase(std::remove_if(components.begin(), components.end(),
                                        [component](const Component& c) {
                                            return c.m_component.get() ==
                                                   component;
                                        }),
                         components.end());

        if (components.empty()) {
            m_components.erase(it);
        }
    }

    [[nodiscard]] bool IsEnable(EntityT entity, T* component) const {
        TL_RETURN_FALSE_IF_NULL(component);

        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return false;
        }

        for (const auto& c : it->second) {
            if (c.m_component.get() == component) {
                return c.m_enable;
            }
        }
        return false;
    }

    void Enable(EntityT entity, uint32_t index) {
        auto it = m_components.find(entity);
        if (it == m_components.end() || index >= it->second.size()) {
            return;
        }
        it->second[index].m_enable = true;
    }

    void Enable(EntityT entity, T* component) {
        TL_RETURN_IF_NULL(component);

        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return;
        }

        for (auto& c : it->second) {
            if (c.m_component.get() == component) {
                c.m_enable = true;
                return;
            }
        }
    }

    void EnableAll(EntityT entity) {
        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return;
        }

        for (auto& c : it->second) {
            c.m_enable = true;
        }
    }

    void Disable(EntityT entity, uint32_t index) {
        auto it = m_components.find(entity);
        if (it == m_components.end() || index >= it->second.size()) {
            return;
        }
        it->second[index].m_enable = false;
    }

    void Disable(EntityT entity, T* component) {
        TL_RETURN_IF_NULL(component);

        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return;
        }

        for (auto& c : it->second) {
            if (c.m_component.get() == component) {
                c.m_enable = false;
                return;
            }
        }
    }

    void DisableAll(EntityT entity) {
        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return;
        }

        for (auto& c : it->second) {
            c.m_enable = false;
        }
    }

    [[nodiscard]] size_t GetComponentSize(EntityT entity) const {
        auto it = m_components.find(entity);
        if (it == m_components.end()) {
            return 0;
        }
        return it->second.size();
    }

    [[nodiscard]] bool Has(EntityT entity) const {
        return m_components.find(entity) != m_components.end();
    }

    [[nodiscard]] expose_type Get(EntityT entity, uint32_t index) const {
        auto it = m_components.find(entity);
        if (it == m_components.end() || index >= it->second.size()) {
            return nullptr;
        }
        return it->second[index].m_component.get();
    }

    [[nodiscard]] expose_type Get(EntityT entity, uint32_t index) {
        return const_cast<expose_type>(std::as_const(*this).Get(entity, index));
    }

    void Clear() { m_components.clear(); }

protected:
    struct Component {
        component_type m_component;
        bool m_enable = true;
    };

    std::unordered_map<EntityT, std::vector<Component>> m_components;
};
