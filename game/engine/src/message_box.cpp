#include "engine/message_box.hpp"
#include "SDL3/SDL_messagebox.h"
#include "engine/sdl_call.hpp"

MessageBox::MessageBox(const std::string& title, const std::string& msg,
                       Type type)
    : m_title{title}, m_msg{msg}, m_type{type} {}

MessageBox::ButtonID MessageBox::AddButton(const std::string& text) {
    ButtonData data;
    data.m_name = text;
    data.m_id = m_btn_id++;
    return m_btn_datas.emplace_back(data).m_id;
}

MessageBox::ButtonID MessageBox::AddReturnButton(const std::string& text) {
    ButtonData data;
    data.m_name = text;
    data.m_id = m_btn_id++;
    data.m_flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    return m_btn_datas.emplace_back(data).m_id;
}

MessageBox::ButtonID MessageBox::AddEscapeButton(const std::string& text) {
    ButtonData data;
    data.m_name = text;
    data.m_id = m_btn_id++;
    data.m_flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    return m_btn_datas.emplace_back(data).m_id;
}

MessageBox::ButtonID MessageBox::Show() {
    SDL_MessageBoxData data{};
    data.title = m_title.c_str();
    data.message = m_msg.c_str();

    std::vector<SDL_MessageBoxButtonData> btn_datas;
    for (auto& btn : m_btn_datas) {
        SDL_MessageBoxButtonData btn_data;
        btn_data.buttonID = btn.m_id;
        btn_data.text = btn.m_name.c_str();
        btn_data.flags = btn.m_flags;
        btn_datas.push_back(btn_data);
    }
    data.buttons = btn_datas.data();
    data.numbuttons = m_btn_datas.size();
    switch (m_type) {
        case Type::Error:
            data.flags = SDL_MESSAGEBOX_ERROR;
            break;
        case Type::Warn:
            data.flags = SDL_MESSAGEBOX_WARNING;
            break;
        case Type::Info:
            data.flags = SDL_MESSAGEBOX_INFORMATION;
            break;
    }
    int btn;
    SDL_CALL(SDL_ShowMessageBox(&data, &btn));
    return btn;
}
