#pragma once

#include "SDL3/SDL.h"
#include <string>
#include <vector>

class MessageBox {
public:
    using ButtonID = int;

    enum class Type {
        Error,
        Warn,
        Info,
    };

    MessageBox(const std::string& title, const std::string& msg, Type type);
    ButtonID Show();
    ButtonID AddButton(const std::string& text);
    ButtonID AddReturnButton(const std::string& text);
    ButtonID AddEscapeButton(const std::string& text);

private:
    struct ButtonData {
        std::string m_name;
        int m_id = -1;
        SDL_MessageBoxButtonFlags m_flags{};
    };

    ButtonID m_btn_id = 0;
    ButtonID m_pressed_button = -1;
    std::string m_title;
    std::string m_msg;
    Type m_type;

    std::vector<ButtonData> m_btn_datas;
};
