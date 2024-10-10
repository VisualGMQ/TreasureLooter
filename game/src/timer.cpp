#include "timer.hpp"
#include "macro.hpp"

namespace tl {

Time::Time() {
    fpsStatistic_.fill(0);
}

void Time::BeginRecordElapse() {
    elapseTime_ = SDL_GetTicks64();
}

void Time::EndRecordElapse() {
    elapseTime_ = SDL_GetTicks64() - elapseTime_;
    lastElapseTime_ = elapseTime_;

    for (int i = 0; i < fpsStatistic_.size() - 1; i++) { 
        fpsStatistic_[i] = fpsStatistic_[i + 1];
    }
    fpsStatistic_.back() = elapseTime_;
}

TimeType Time::GetElapse() const {
    return lastElapseTime_;
}

uint32_t Time::GetFPS() const {
    if (lastElapseTime_ == 0) {
        return std::numeric_limits<uint32_t>::max();
    }
    return 1000 / lastElapseTime_;
}

uint32_t Time::GetAverageFPS() const {
    float sum = 0;
    for (int i = 0; i < fpsStatistic_.size(); i++) {
        sum += fpsStatistic_[i];
    }

    return uint32_t(1000.0f / (sum / fpsStatistic_.size()));
}

Timer::Timer(TimeType interval, int loop, Timer::Callback callback)
    : interval_{interval}, loop_{loop}, callback_{callback} {}

void Timer::Update(TimeType elapse) {
    TL_RETURN_IF_FALSE(isWorking_);

    curTime_ += elapse;
    if (curTime_ >= interval_) {
        curTime_ -= interval_;
        if (callback_) {
            interval_ = callback_(*this);
        }

        if (loop_ == 0) {
            isWorking_ = false;
        }

        if (loop_ > 0) {
            loop_--;
        }
    }
}

void Timer::Pause() {
    isWorking_ = false;
}

void Timer::Start() {
    isWorking_ = true;
}

TimerID TimerManager::Create(TimeType interval, int loop,
                             Timer::Callback callback) {
    auto result = timers_.emplace(++curID_, Timer{interval, loop, callback});
    TL_RETURN_VALUE_IF_FALSE(result.second, TimerID{});
    return result.first->first;
}

void TimerManager::Destroy(TimerID id) {
    timers_.erase(id);
}

const Timer* TimerManager::Find(TimerID id) const {
    auto it = timers_.find(id.id_);
    if (it != timers_.end()) {
        return &it->second;
    }
    return nullptr;
}

Timer* TimerManager::Find(TimerID id) {
    return const_cast<Timer*>(std::as_const(*this).Find(id.id_));
}

void TimerManager::Update(TimeType elapse) {
    for (auto& [id, timer] : timers_) {
        timer.Update(elapse);
    }
}

}  // namespace tl