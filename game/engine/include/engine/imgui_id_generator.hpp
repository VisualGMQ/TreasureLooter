#pragma once
#include <cstdint>

class ImGuiIDGenerator {
public:
    static int Gen() { return id++; }
    static void Reset() { id = 0; }

private:
    static int id;
};