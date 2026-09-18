#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>

#include "common/event.hpp"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

#include "schema/timer_event.hpp"

/**
 * in seconds
 */
using TimeType = double;

constexpr uint32_t kNoLimitFPS = 0;

class Time {
public:
    Time();

    void Update();
    [[nodiscard]] TimeType GetCurrentTime() const;

    [[nodiscard]] TimeType GetElapseTime() const;
    [[nodiscard]] uint32_t GetFPS() const;

    [[nodiscard]] uint32_t GetUnlimitFPS() const { return m_unlimit_fps; }

    void SetFPS(uint32_t fps);
    [[nodiscard]] bool IsFPSLimited() const;

    void Begin();
    void End();

private:
    static constexpr TimeType MinElapseTime = 0.000001;

    // for whole frame
    TimeType m_elapsed_time{MinElapseTime};
    std::chrono::steady_clock::time_point m_cur_time{};

    // for fps limit
    std::chrono::steady_clock::time_point m_cur_frame_begin_time{};
    uint32_t m_limit_fps = kNoLimitFPS;
    float m_fps_require_time = 0.0;  // in ms
    uint32_t m_unlimit_fps = 0;
};

enum class TimerID : uint32_t {};

std::ostream& operator<<(std::ostream& o, TimerID);

// for spdlog output
template <>
struct fmt::formatter<TimerID> : fmt::ostream_formatter {};

struct NullTimerID {
    constexpr bool operator==(NullTimerID) const { return true; }

    constexpr bool operator!=(NullTimerID) const { return false; }

    constexpr bool operator==(TimerID id) const {
        return static_cast<std::underlying_type_t<TimerID>>(id) == 0;
    }

    constexpr bool operator!=(TimerID id) const { return !(*this == id); }

    operator TimerID() const { return static_cast<TimerID>(0); }
};

constexpr bool operator==(TimerID id, NullTimerID null) {
    return null == id;
}

constexpr bool operator!=(TimerID id, NullTimerID null) {
    return null != id;
}

constexpr NullTimerID null_timer_id;

class Timer;

class TimerEvent {
public:
    TimerEvent(TimerEventType, Timer&);

    [[nodiscard]] TimerEventType GetEventType() const;
    [[nodiscard]] Timer& GetTimer() const;

private:
    TimerEventType m_type;
    Timer& m_timer;
};

class TimerStopEvent {
public:
    TimerStopEvent(TimerEventType, Timer&);

    [[nodiscard]] TimerEventType GetEventType() const;
    [[nodiscard]] Timer& GetTimer() const;

private:
    TimerEventType m_type;
    Timer& m_timer;
};

class Timer {
public:
    using TimerListener = std::function<void(const TimerEvent&)>;
    using TimerStopListener = std::function<void(const TimerStopEvent&)>;

    Timer() = default;

    Timer(const Timer&) = delete;

    Timer& operator=(const Timer&) = delete;

    Timer(Timer&&) = default;

    Timer& operator=(Timer&&) = default;

    explicit Timer(TimerID id, TimeType time, TimerEventType event_type,
                   int loop);
    ~Timer();

    void SetInterval(TimeType interval);

    void SetTimerListener(const TimerListener&);
    void SetTimerStopListener(const TimerStopListener&);

    /**
     * will take event listener ownership
     */
    void SetTimerListener(EventListenerID);

    /**
     * will take event listener ownership
     */
    void SetTimerStopListener(EventListenerID);

    void Update(TimeType);

    void Start();

    void Stop();

    void Rewind();

    void SetLoop(int loop);

    void Pause();

    [[nodiscard]] TimeType GetInterval() const;

    [[nodiscard]] TimerEventType GetEventType() const;

    void SetEventType(TimerEventType);
    [[nodiscard]] TimerID GetID() const;
    [[nodiscard]] bool IsRunning() const;

private:
    TimerID m_id = null_timer_id;
    EventListenerID m_timer_event_listener_id = null_event_listener_id;
    EventListenerID m_timer_stop_event_listener_id = null_event_listener_id;
    bool m_is_running{false};
    TimeType m_cur_time{};
    TimeType m_interval{};
    TimerEventType m_event_type = TimerEventType::Unknown;
    int m_loop = 0;
    int m_cur_loop = 0;
};

class TimerManager {
public:
    Timer& Create(TimeType interval, TimerEventType event_type, int loop = 0);

    void Remove(TimerID);
    void Remove(const Timer& timer);

    void Clear();

    void Update(TimeType);

    [[nodiscard]] Timer* Find(TimerID) const;

private:
    std::underlying_type_t<TimerID> m_cur_id = 0;
    std::unordered_map<TimerID, std::unique_ptr<Timer>> m_timers;
};
