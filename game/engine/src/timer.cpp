#include "engine/timer.hpp"

#include "engine/context.hpp"
#include "engine/profile.hpp"

Time::Time() {
    m_cur_time = std::chrono::steady_clock::now();
}

void Time::Update() {
    PROFILE_SECTION();

    auto cur_time = std::chrono::steady_clock::now();
    auto elapsed_time = cur_time - m_cur_time;
    m_cur_time = cur_time;
    m_elapsed_time =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time)
            .count() /
        1e6;

    m_elapsed_time = std::max(m_elapsed_time, MinElapseTime);
}

TimeType Time::GetCurrentTime() const {
    auto cur_time = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
               cur_time.time_since_epoch())
               .count() /
           1e6;
}

TimeType Time::GetElapseTime() const {
    return m_elapsed_time;
}

void Time::SetFPS(float fps) {
    m_limit_fps = fps;
    m_fps_require_time = 1000.0 / fps;
}

bool Time::IsFPSLimited() const {
    return m_limit_fps != kNoLimitFPS;
}

void Time::Begin() {
    TL_RETURN_IF_FALSE(m_limit_fps != kNoLimitFPS);
    
    m_cur_frame_begin_time = std::chrono::steady_clock::now();
}

void Time::End() {
    TL_RETURN_IF_FALSE(m_limit_fps != kNoLimitFPS);
    
    auto elapse = std::chrono::steady_clock::now() - m_cur_frame_begin_time;
    float elapse_time= std::chrono::duration_cast<std::chrono::milliseconds>(elapse).count();
    
    TL_RETURN_IF_FALSE(elapse_time < m_fps_require_time);
    
    SDL_Delay(m_fps_require_time - elapse_time);
}

TimerEvent::TimerEvent(TimerEventType type, TimerID id)
    : m_type{type}, m_timer_id{id} {}

TimerEventType TimerEvent::GetEventType() const {
    return m_type;
}

TimerID TimerEvent::GetID() const {
    return m_timer_id;
}

TimerStopEvent::TimerStopEvent(TimerEventType type, TimerID id)
    : m_type{type}, m_timer_id{id} {}

TimerEventType TimerStopEvent::GetEventType() const {
    return m_type;
}

TimerID TimerStopEvent::GetID() const {
    return m_timer_id;
}

Timer::Timer(TimerID id, TimeType time, TimerEventType event_type, int loop)
    : m_id{id} {
    SetInterval(time);
    SetEventType(event_type);
    SetLoop(loop);
}

void Timer::SetInterval(TimeType interval) {
    m_interval = interval;
}

void Timer::Update(TimeType time) {
    if (!m_is_running || m_interval == 0) {
        return;
    }

    m_cur_time += time;

    if (m_cur_loop == 0) {
        if (m_cur_time >= m_interval) {
            CURRENT_CONTEXT.m_event_system->EnqueueEvent<TimerEvent>(
                TimerEvent{m_event_type, m_id});
            Stop();
            CURRENT_CONTEXT.m_event_system->EnqueueEvent<TimerStopEvent>(
                TimerStopEvent{m_event_type, m_id});
        }
    } else {
        while (m_cur_loop != 0 && m_cur_time >= m_interval) {
            m_cur_time -= m_interval;
            if (m_cur_loop > 0) {
                m_cur_loop--;
            }
            CURRENT_CONTEXT.m_event_system->EnqueueEvent<TimerEvent>(
                TimerEvent{m_event_type, m_id});
        }
    }
}

void Timer::Start() {
    m_is_running = true;
}

void Timer::Stop() {
    Pause();
    Rewind();
}

void Timer::Rewind() {
    m_cur_time = 0;
    m_cur_loop = m_loop;
}

void Timer::Pause() {
    m_is_running = false;
}

void Timer::SetLoop(int loop) {
    m_loop = loop;
    m_cur_loop = m_loop;
}

TimeType Timer::GetInterval() const {
    return m_interval;
}

TimerEventType Timer::GetEventType() const {
    return m_event_type;
}

void Timer::SetEventType(TimerEventType type) {
    m_event_type = type;
}

TimerID Timer::GetID() const {
    return m_id;
}

bool Timer::IsRunning() const {
    return m_is_running;
}

Timer& TimerManager::Create(TimeType interval, TimerEventType event_type,
                            int loop) {
    auto id = static_cast<TimerID>(++m_cur_id);
    return *m_timers
                .emplace(
                    id, std::make_unique<Timer>(id, interval, event_type, loop))
                .first->second;
}

void TimerManager::Remove(TimerID timer) {
    m_timers.erase(timer);
}

void TimerManager::Clear() {
    m_timers.clear();
}

void TimerManager::Update(TimeType duration) {
    PROFILE_SECTION();

    for (auto& [_, timer] : m_timers) {
        timer->Update(duration);
    }
}

Timer* TimerManager::Find(TimerID timer) const {
    if (auto it = m_timers.find(timer); it != m_timers.end()) {
        return it->second.get();
    }
    return nullptr;
}
