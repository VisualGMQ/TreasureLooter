#pragma once
#include <chrono>

using TimeType = double;

class Time {
public:
    Time();
    
    void Update();

    /**
     * get elapsed time between two frames
     * @return time_type in milliseconds
     */
    TimeType GetElapseTime() const;

private:
    static constexpr TimeType MinElapseTime = 0.000001;
    
    TimeType m_elapsed_time{MinElapseTime};
    std::chrono::steady_clock::time_point m_cur_time{};
};