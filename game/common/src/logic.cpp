#include "common/logic.hpp"

Vec2 CalcPlankPos(int side) {
    auto logic_size = COMMON_CONTEXT.GetGameConfig().m_logic_size;

    auto half_win_size = Vec2{logic_size} * 0.5;
    if (side < 0) {
        return {-half_win_size.w + RectHalfSize.w, 0};
    }
    return {half_win_size.w - RectHalfSize.w, 0};
}