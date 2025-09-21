#pragma once
#include <chrono>
#include <memory>
#include <unordered_map>

#include "schema/timer_event.hpp"

using TimeType = double;

class Time {
public:
    Time();

    void Update();
    TimeType GetCurrentTime() const;

    /**
     * get elapsed time between two frames
     * @return time_type in milliseconds
     */
    [[nodiscard]] TimeType GetElapseTime() const;

private:
    static constexpr TimeType MinElapseTime = 0.000001;

    TimeType m_elapsed_time{MinElapseTime};
    std::chrono::steady_clock::time_point m_cur_time{};
};

enum class TimerID: uint32_t {
};

struct NullTimerID {
    constexpr bool operator==(NullTimerID) const { return true; }
    constexpr bool operator!=(NullTimerID) const { return false; }

    constexpr bool operator==(TimerID id) const {
        return static_cast<std::underlying_type_t<TimerID>>(id) == 0;
    }

    constexpr bool operator!=(TimerID id) const { return !(*this == id); }
    operator TimerID() const { return static_cast<TimerID>(0); }
};

constexpr bool operator==(TimerID id, NullTimerID null) { return null == id; }
constexpr bool operator!=(TimerID id, NullTimerID null) { return null != id; }

constexpr NullTimerID null_timer_id;

class TimerEvent {
public:
    TimerEvent(TimerEventType type, TimerID);

    [[nodiscard]] TimerEventType GetType() const;

private:
    TimerEventType m_type;
    TimerID m_timer_id = null_timer_id;
};

class Timer {
public:
    Timer() = default;

    Timer(const Timer&) = delete;

    Timer& operator=(const Timer&) = delete;

    Timer(Timer&&) = default;

    Timer& operator=(Timer&&) = default;

    explicit Timer(TimerID id, TimeType time, TimerEventType event_type,
                   int loop);

    void SetInterval(TimeType interval);

    void Update(TimeType);

    void Start();

    void Stop();

    void Rewind();

    void SetLoop(int loop);

    void Pause();

    [[nodiscard]] TimeType GetInterval() const;

    [[nodiscard]] TimerEventType GetEventType() const;

    void SetEventType(TimerEventType);

private:
    TimerID m_id = null_timer_id;
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

    void RemoveTimer(TimerID);

    void Clear();

    void Update(TimeType);

    [[nodiscard]] Timer* Find(TimerID) const;

private:
    std::underlying_type_t<TimerID> m_cur_id = 0;
    std::unordered_map<TimerID, std::unique_ptr<Timer> > m_timers;
};
