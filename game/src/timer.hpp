#pragma once
#include "id.hpp"
#include "pch.hpp"
#include "common.hpp"

namespace tl {

class Time {
public:
    Time();
    TimeType GetElapse() const;
    uint32_t GetFPS() const;
    uint32_t GetAverageFPS() const;
    void BeginRecordElapse();
    void EndRecordElapse();

    Time(const Time&) = delete;
    Time(Time&&) = delete;
    Time& operator=(Time&&) = delete;
    Time& operator=(const Time&) = delete;

private:
    TimeType elapseTime_ = 0;
    TimeType lastElapseTime_ = 0;

    std::array<float, 100> fpsStatistic_;
};

class Timer;
class TimerManager;

using TimerID = ID<Timer, TimerManager>;

class Timer {
public:
    friend class TimerManager;

    static constexpr int InfLoop = -1;

    using Callback = std::function<TimeType(Timer&)>;

    Timer(TimeType interval, int loop, Callback callback);

    Timer(Timer&&) = default;
    Timer& operator=(Timer&&) = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    int GetRemainLoop() const { return loop_; }

    TimeType GetInterval() const { return interval_; }

    TimeType GetTime() const { return curTime_; }

    TimeType GetRemainTime() const { return interval_ - curTime_; }

    void Update(TimeType elapse);
    void Pause();
    void Start();

    void ChangeCallback(Callback cb) { callback_ = cb; }

private:
    TimeType curTime_ = 0;
    TimeType interval_;
    Callback callback_;
    int loop_;
    bool isWorking_ = true;
};

class TimerManager {
public:
    TimerID Create(TimeType interval, int loop, Timer::Callback callback);
    void Destroy(TimerID);
    const Timer* Find(TimerID) const;
    Timer* Find(TimerID);
    void Update(TimeType);

private:
    std::unordered_map<TimerID::underlying_type, Timer> timers_;
    TimerID::underlying_type curID_ = 0;
};

}  // namespace tl