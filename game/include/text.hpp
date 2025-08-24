#pragma once
#include "path.hpp"

class Font {
public:
    explicit Font(const Path& filename);
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    std::vector<char> GenerateText(const std::string& text, uint32_t pixel) const;

private:
};