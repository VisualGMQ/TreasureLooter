#include "common.hpp"

namespace tl {

uint32_t ParseFloat(std::string_view text, float* values, uint32_t count) {
    size_t begin = 0, end = 0;
    while (count > 0 && begin < text.size() && end < text.size()) {
        if (text[begin] == ' ') {
            begin++;
        } else {
            end = begin;
            while (end < text.size() && text[end] != ' ') {
                end++;
            }

            *values = std::atof(text.data() + begin);
            values++;
            count--;

            begin = end;
        }
    }

    return count;
}

}