#include "common/timer.hpp"

#include "common/context.hpp"
#include "common/event.hpp"
#include "common/profile.hpp"

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

uint32_t Time::GetFPS() const {
    if (m_elapsed_time <= 0) return 0;
    return static_cast<uint32_t>(1.0 / m_elapsed_time);
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
    float elapse_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapse).count();

    if (elapse_time > 0) {
        m_unlimit_fps = static_cast<uint32_t>(1000.0 / elapse_time);
    }

    TL_RETURN_IF_FALSE(elapse_time < m_fps_require_time);

    SDL_Delay(m_fps_require_time - elapse_time);
}

std::ostream& operator<<(std::ostream& o, TimerID id) {
    o << "TimerID(" << static_cast<std::underlying_type_t<TimerID>>(id) << ")";
    return o;
}

TimerEvent::TimerEvent(TimerEventType type, Timer& timer)
    : m_type{type}, m_timer{timer} {}

TimerEventType TimerEvent::GetEventType() const {
    return m_type;
}

Timer& TimerEvent::GetTimer() const {
    return m_timer;
}

TimerStopEvent::TimerStopEvent(TimerEventType type, Timer& timer)
    : m_type{type}, m_timer{timer} {}

TimerEventType TimerStopEvent::GetEventType() const {
    return m_type;
}

Timer& TimerStopEvent::GetTimer() const {
    return m_timer;
}

Timer::Timer(TimerID id, TimeType time, TimerEventType event_type, int loop)
    : m_id{id} {
    SetInterval(time);
    SetEventType(event_type);
    SetLoop(loop);
}

Timer::~Timer() {
    COMMON_CONTEXT.m_event_system->RemoveListener<TimerEvent>(
        m_timer_event_listener_id);
    COMMON_CONTEXT.m_event_system->RemoveListener<TimerStopEvent>(
        m_timer_stop_event_listener_id);
}

void Timer::SetInterval(TimeType interval) {
    m_interval = interval;
}

void Timer::SetTimerListener(const TimerListener& listener) {
    m_timer_event_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<TimerEvent>(
            [listener, this](EventListenerID id, const TimerEvent& event) {
                TL_RETURN_IF_FALSE(id == this->m_timer_event_listener_id);
                listener(event);
            });
}

void Timer::SetTimerStopListener(const TimerStopListener& listener) {
    m_timer_event_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<TimerStopEvent>(
            [listener, this](EventListenerID id, const TimerStopEvent& event) {
                TL_RETURN_IF_FALSE(id == this->m_timer_stop_event_listener_id);
                listener(event);
            });
}

void Timer::SetTimerListener(EventListenerID id) {
    m_timer_event_listener_id = id;
}

void Timer::SetTimerStopListener(EventListenerID id) {
    m_timer_stop_event_listener_id = id;
}

void Timer::Update(TimeType time) {
    if (!m_is_running || m_interval == 0) {
        return;
    }

    m_cur_time += time;

    if (m_cur_loop == 0) {
        if (m_cur_time >= m_interval) {
            COMMON_CONTEXT.m_event_system->EnqueueEvent<TimerEvent>(
                TimerEvent{m_event_type, *this});
            Stop();
            COMMON_CONTEXT.m_event_system->EnqueueEvent<TimerStopEvent>(
                TimerStopEvent{m_event_type, *this});
        }
    } else {
        while (m_cur_loop != 0 && m_cur_time >= m_interval) {
            m_cur_time -= m_interval;
            if (m_cur_loop > 0) {
                m_cur_loop--;
            }
            COMMON_CONTEXT.m_event_system->EnqueueEvent<TimerEvent>(
                TimerEvent{m_event_type, *this});
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

void TimerManager::Remove(const Timer& timer) {
    Remove(timer.GetID());
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
