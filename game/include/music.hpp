#pragma once

#include "path.hpp"

class Sound {
public:
    explicit Sound(const Path& filename);
    ~Sound();

private:
    short* m_data{};
};