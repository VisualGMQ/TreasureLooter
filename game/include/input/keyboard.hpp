#pragma once
#include "SDL3/SDL.h"
#include "input/button.hpp"
#include "schema/key.hpp"

#include <unordered_map>

class KeyboardButton : public Button {
public:
    friend class Keyboard;
    
    explicit KeyboardButton(Key key);

    bool IsPressing() const override;
    bool IsReleasing() const override;
    bool IsReleased() const override;
    bool IsPressed() const override;
   
private:
    Key m_key;
    bool m_is_press = false;
    bool m_is_last_frame_press = false;
    bool m_has_handled_event = false;
    
    void handleEvent(const SDL_KeyboardEvent&);
    void update();
};

class Keyboard {
public:
    void HandleEvent(const SDL_KeyboardEvent&);
    void Update();

    const KeyboardButton& Get(Key);
    
private:
    std::unordered_map<Key, KeyboardButton> m_buttons;
};