#pragma once
#include "common/animation.hpp"
#include "schema/anim_player.hpp"

enum class AnimationPlayerID : uint32_t {};
inline constexpr AnimationPlayerID null_animation_player_id =
    static_cast<AnimationPlayerID>(0);

class AnimationTrackPlayerBase {
public:
    virtual ~AnimationTrackPlayerBase() = default;
    virtual void Update(TimeType) = 0;
    virtual void Rewind() = 0;
    [[nodiscard]] virtual AnimationTrackType GetType() const = 0;
    [[nodiscard]] virtual TimeType GetFinishTime() const = 0;
    [[nodiscard]] virtual bool IsComplete() const = 0;
};

template <typename T>
class IAnimationTrackPlayer : public AnimationTrackPlayerBase {
public:
    using keyframe_type = KeyFrame<T>;

    explicit IAnimationTrackPlayer(const IAnimationTrack<T>& track)
        : m_track(track) {}

    void Rewind() override {
        m_cur_frame = -1;
        m_cur_time = 0;
    }

    void Update(TimeType delta_time) override {
        auto& keyframes = m_track.GetKeyframes();
        if (keyframes.empty()) {
            return;
        }
        if (m_cur_frame + 1 >= static_cast<int>(keyframes.size())) {
            return;
        }

        // One delta per call; then advance across any keyframe boundaries
        // crossed. (The old loop added delta_time every iteration, so a single
        // large seek step multiplied by the number of crossed keyframes and
        // jumped to the end of the track.)
        m_cur_time += delta_time;
        while (m_cur_frame + 1 < static_cast<int>(keyframes.size())) {
            const auto& next_frame = keyframes[m_cur_frame + 1];
            if (next_frame.m_time <= m_cur_time) {
                m_cur_frame++;
            } else {
                break;
            }
        }

        m_cur_time = std::min(keyframes.back().m_time, m_cur_time);
    }

    virtual T GetValue() const = 0;

    [[nodiscard]] bool NeedSync() const { return m_cur_frame >= 0; }

    [[nodiscard]] bool IsComplete() const override {
        return m_cur_frame + 1 >=
               static_cast<int>(m_track.GetKeyframes().size());
    }

    auto& GetTrack() const { return m_track; }

    [[nodiscard]] AnimationTrackType GetType() const override {
        return m_track.GetType();
    }

    [[nodiscard]] TimeType GetFinishTime() const override {
        return m_track.GetFinishTime();
    }

protected:
    const IAnimationTrack<T>& m_track;
    TimeType m_cur_time{};
    int m_cur_frame = -1;
};

template <typename T, AnimationTrackType Type>
class AnimationTrackPlayer;

template <typename T>
class AnimationTrackPlayer<T, AnimationTrackType::Linear>
    : public IAnimationTrackPlayer<T> {
public:
    using track_type = AnimationTrack<T, AnimationTrackType::Linear>;

    explicit AnimationTrackPlayer(const track_type& track)
        : IAnimationTrackPlayer<T>(track) {}

    T GetValue() const override {
        auto& track = static_cast<const track_type&>(this->GetTrack());
        auto& keyframes = track.GetKeyframes();

        if (keyframes.empty()) {
            return {};
        }

        if (this->m_cur_frame + 1 >= keyframes.size()) {
            return keyframes[this->m_cur_frame].m_value;
        }
        auto& cur_frame = keyframes[this->m_cur_frame];
        auto& next_frame = keyframes[this->m_cur_frame + 1];
        float t = (this->m_cur_time - cur_frame.m_time) /
                  (next_frame.m_time - cur_frame.m_time);
        return Lerp(cur_frame.m_value, next_frame.m_value, t);
    }
};

template <typename T>
class AnimationTrackPlayer<T, AnimationTrackType::Discrete>
    : public IAnimationTrackPlayer<T> {
public:
    using track_type = AnimationTrack<T, AnimationTrackType::Discrete>;

    using IAnimationTrackPlayer<T>::IAnimationTrackPlayer;

    T GetValue() const override {
        auto& track = static_cast<const track_type&>(this->GetTrack());
        auto& keyframes = track.GetKeyframes();

        if (keyframes.empty()) {
            return {};
        }
        return keyframes[this->m_cur_frame].m_value;
    }
};

class AnimationPlayer {
public:
    static constexpr int InfLoop = -1;

    template <typename Class, typename T>
    using BindingPointPropertyGetterFn = T& (*)(Class&);

    template <typename T, typename PropertyType,
              AnimationBindingPoint binding_point,
              AnimationTrackType track_type>
    static void RegisterTrackInfo(
        ComponentManager<T>& manager,
        BindingPointPropertyGetterFn<T, PropertyType> property_getter) {
        TL_ASSERT(property_getter);
        auto sync_fn = [=, &manager](void* entity,
                                     AnimationTrackPlayerBase& track_player) {
            TL_ASSERT(entity);
            T* component = manager.Get(*static_cast<Entity*>(entity));
            TL_RETURN_IF_NULL(component);

            auto& raw_track = static_cast<const AnimationTrackPlayer<
                std::decay_t<PropertyType>, track_type>&>(track_player);
            if (raw_track.NeedSync()) {
                property_getter(*component) = raw_track.GetValue();
            }
        };

        auto create_fn = [](AnimationPlayer& player,
                            AnimationTrackBase& track_base) {
            auto& raw_track =
                static_cast<AnimationTrack<PropertyType, track_type>&>(
                    track_base);
            player.m_track_players[binding_point] = std::make_unique<
                AnimationTrackPlayer<PropertyType, track_type>>(raw_track);
        };

        auto [it, success] = g_track_infos.try_emplace(binding_point);
        if constexpr (track_type == AnimationTrackType::Linear) {
            it->second.m_linear_sync_function = sync_fn;
            it->second.m_linear_create_function = create_fn;
        } else {
            it->second.m_discrete_sync_function = sync_fn;
            it->second.m_discrete_create_function = create_fn;
        }
    }

    template <typename T, typename PropertyType,
              AnimationBindingPoint binding_point>
    static void RegisterDiscreteTrackInfo(
        ComponentManager<T>& manager,
        BindingPointPropertyGetterFn<T, PropertyType> property_getter) {
        RegisterTrackInfo<T, PropertyType, binding_point,
                          AnimationTrackType::Discrete>(manager,
                                                        property_getter);
    }

    template <typename T, typename PropertyType,
              AnimationBindingPoint binding_point>
    static void RegisterLinearTrackInfo(
        ComponentManager<T>& manager,
        BindingPointPropertyGetterFn<T, PropertyType> property_getter) {
        RegisterTrackInfo<T, PropertyType, binding_point,
                          AnimationTrackType::Linear>(manager, property_getter);
    }

    template <typename T, typename PropertyType,
              AnimationBindingPoint binding_point>
    static void RegisterLinearDiscreteTrackInfo(
        ComponentManager<T>& manager,
        BindingPointPropertyGetterFn<T, PropertyType> property_getter) {
        RegisterLinearTrackInfo<T, PropertyType, binding_point>(
            manager, property_getter);
        RegisterDiscreteTrackInfo<T, PropertyType, binding_point>(
            manager, property_getter);
    }

    template <typename T, AnimationBindingPoint binding_point,
              AnimationTrackType track_type>
    static void RegisterTrackInfo() {
        auto sync_fn = [=](void* payload,
                           AnimationTrackPlayerBase& track_player) {
            TL_ASSERT(payload);
            auto& raw_track =
                static_cast<const AnimationTrackPlayer<T, track_type>&>(
                    track_player);
            if (raw_track.NeedSync()) {
                *static_cast<T*>(payload) = raw_track.GetValue();
            }
        };

        auto create_fn = [](AnimationPlayer& player,
                            AnimationTrackBase& track_base) {
            auto& raw_track =
                static_cast<AnimationTrack<T, track_type>&>(track_base);
            player.m_track_players[binding_point] =
                std::make_unique<AnimationTrackPlayer<T, track_type>>(
                    raw_track);
        };

        auto [it, success] = g_track_infos.try_emplace(binding_point);
        if constexpr (track_type == AnimationTrackType::Linear) {
            it->second.m_linear_sync_function = sync_fn;
            it->second.m_linear_create_function = create_fn;
        } else {
            it->second.m_discrete_sync_function = sync_fn;
            it->second.m_discrete_create_function = create_fn;
        }
    }

    template <typename T, AnimationBindingPoint binding_point>
    static void RegisterDiscreteTrackInfo() {
        RegisterTrackInfo<T, binding_point, AnimationTrackType::Discrete>();
    }

    template <typename T, AnimationBindingPoint binding_point>
    static void RegisterLinearTrackInfo() {
        RegisterTrackInfo<T, binding_point, AnimationTrackType::Linear>();
    }

    template <typename T, AnimationBindingPoint binding_point>
    static void RegisterLinearDiscreteTrackInfo() {
        RegisterLinearTrackInfo<T, binding_point>();
        RegisterDiscreteTrackInfo<T, binding_point>();
    }


    AnimationPlayer();
    explicit AnimationPlayer(const AnimationPlayerDefinition&);

    AnimationPlayer(AnimationPlayer&&) noexcept = default;
    AnimationPlayer& operator=(AnimationPlayer&&) noexcept = default;

    [[nodiscard]] AnimationPlayerID GetID() const;

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    void SetLoop(int);
    [[nodiscard]] bool IsPlaying() const;

    [[nodiscard]] int GetLoopCount() const;
    [[nodiscard]] TimeType GetCurTime() const;
    [[nodiscard]] TimeType GetMaxTime() const;

    void ChangeAnimation(const Path& filename);
    void ChangeAnimation(AnimationHandle);
    void ChangeAnimation(UUIDv4);
    void ClearAnimation();
    [[nodiscard]] bool HasAnimation() const;

    void SetCurTime(TimeType time);

    void Update(TimeType delta_time);
    void Sync(Entity entity);

    void SetRate(float rate);
    [[nodiscard]] float GetRate() const;

    [[nodiscard]] AnimationHandle GetAnimation() const;

    void EnableAutoPlay(bool enable);
    [[nodiscard]] bool IsAutoPlayEnabled() const;

private:
    static std::underlying_type_t<AnimationPlayerID> g_next_id;

    struct TrackInfo {
        using SyncFunction =
            std::function<void(void*, AnimationTrackPlayerBase&)>;
        using TrackCreateFunction =
            std::function<void(AnimationPlayer&, AnimationTrackBase&)>;

        SyncFunction m_linear_sync_function;
        SyncFunction m_discrete_sync_function;
        TrackCreateFunction m_linear_create_function;
        TrackCreateFunction m_discrete_create_function;
    };

    static std::unordered_map<AnimationBindingPoint, TrackInfo> g_track_infos;

    AnimationPlayerID m_id = null_animation_player_id;
    Entity m_entity = null_entity;
    AnimationHandle m_animation;

    std::unordered_map<AnimationBindingPoint,
                       std::unique_ptr<AnimationTrackPlayerBase>>
        m_track_players;
    std::unordered_map<std::string, std::unique_ptr<AnimationTrackPlayerBase>>
        m_bind_point_track_players;

    bool m_auto_play = false;
    bool m_is_playing = false;
    int m_loop{0};
    TimeType m_cur_time{};
    float m_rate = 1.0;
};

class MultiAnimationPlayer {
public:
    MultiAnimationPlayer() = default;

    explicit MultiAnimationPlayer(
        const MultiAnimationPlayerDefinition& definition);

    [[nodiscard]] const AnimationPlayer& GetAnimation(size_t) const;
    [[nodiscard]] AnimationPlayer& GetAnimation(size_t);

    [[nodiscard]] size_t GetAnimationCount() const { return m_players.size(); }

    void AddAnimation(AnimationPlayer&&);
    [[nodiscard]] AnimationPlayer& AddAnimation(
        const AnimationPlayerDefinition&);
    [[nodiscard]] AnimationPlayer& AddAnimation(AnimationHandle);
    void RemoveAnimation(const AnimationPlayer&);

    std::vector<AnimationPlayer>& GetAnimations();
    const std::vector<AnimationPlayer>& GetAnimations() const;

    void Update(TimeType elapse_time);
    void Sync(Entity);

    void PlayAll();
    void PauseAll();
    void StopAll();
    void RewindAll();

private:
    std::vector<AnimationPlayer> m_players;
};

class MultiAnimationPlayerManager
    : public ComponentManager<MultiAnimationPlayer> {
public:
    void Update(TimeType delta_time);
};

class AnimationEndEvent {
public:
    AnimationEndEvent(AnimationPlayerID animation_player_id, Entity entity,
                      AnimationHandle animation)
        : m_animation_player_id(animation_player_id),
          m_entity(entity),
          m_animation(animation) {}

    [[nodiscard]] AnimationPlayerID GetAnimationPlayerID() const {
        return m_animation_player_id;
    }

    [[nodiscard]] Entity GetEntity() const { return m_entity; }

    [[nodiscard]] AnimationHandle GetAnimation() const { return m_animation; }

private:
    AnimationPlayerID m_animation_player_id;
    Entity m_entity;
    AnimationHandle m_animation;
};
