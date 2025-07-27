#include "timer.hpp"

#include "log.hpp"

#include <chrono>

Time::Time() {
    m_cur_time = std::chrono::steady_clock::now();
}

void Time::Update() {
    auto cur_time = std::chrono::steady_clock::now();
    auto elapsed_time = cur_time - m_cur_time;
    m_cur_time = cur_time;
    m_elapsed_time =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time)
            .count() /
        1000000.0;

    m_elapsed_time = std::max(m_elapsed_time, MinElapseTime);
}

TimeType Time::GetElapseTime() const {
    return m_elapsed_time;
}