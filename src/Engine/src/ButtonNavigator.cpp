#include "ButtonNavigator.hpp"

ButtonNavigator::ButtonNavigator(Gui& gui, Configuration& config)
    : gui(gui), config(config), on_cancel(0), on_cancel_data(0),
      joymotion_up(false), joymotion_down(false) { }

ButtonNavigator::~ButtonNavigator() { }

void ButtonNavigator::install_handlers(GuiWindow *window,
    OnCancel on_cancel, void *on_cancel_data)
{
    if (window) {
        window->set_on_keydown(static_window_keydown, this);
        window->set_on_joymotion(static_window_joymotion, this);
        window->set_on_joybuttondown(static_window_joybutton_down, this);
    }
    this->on_cancel = on_cancel;
    this->on_cancel_data = on_cancel_data;
}

void ButtonNavigator::add_button(GuiButton *btn) {
    navigator_buttons.push_back(btn);
    if (current_index < 0) {
        current_index = 0;
    }
}

void ButtonNavigator::clear() {
    navigator_buttons.clear();
    current_index = -1;
}

void ButtonNavigator::set_button_focus() {
    for (NavigatorButtons::iterator it = navigator_buttons.begin();
        it != navigator_buttons.end(); it++)
    {
        (*it)->reset_color();
    }
    if (current_index >= 0 && current_index < static_cast<int>(navigator_buttons.size())) {
        navigator_buttons[current_index]->set_color(1.0f, 0.5f, 0.5f);
    }
}

bool ButtonNavigator::static_window_keydown(GuiWindow *sender, void *data, int keycode, bool repeat) {
    return (reinterpret_cast<ButtonNavigator *>(data))->window_keydown(keycode, repeat);
}

bool ButtonNavigator::window_keydown(int keycode, bool repeat) {
    binding.extract_from_config(config);
    if (binding.down.device == MappedKey::DeviceKeyboard && binding.down.param == keycode) {
        set_button_focus_down();
        return true;
    }
    if (binding.up.device == MappedKey::DeviceKeyboard && binding.up.param == keycode) {
        set_button_focus_up();
        return true;
    }
    if (binding.fire.device == MappedKey::DeviceKeyboard && binding.fire.param == keycode) {
        do_current_button_click();
        return true;
    }
    if (binding.escape.device == MappedKey::DeviceKeyboard && binding.escape.param == keycode) {
        if (on_cancel) {
            on_cancel(on_cancel_data);
        }
        return true;
    }

    return false;
}

bool ButtonNavigator::static_window_joymotion(GuiWindow *sender, void *data, int motion) {
    return (reinterpret_cast<ButtonNavigator *>(data))->window_joymotion(motion);
}

bool ButtonNavigator::window_joymotion(int motion) {
    if (motion & InputData::InputJoyDirectionDown && !joymotion_down) {
        joymotion_down = true;
        set_button_focus_down();
    } else {
        joymotion_down = false;
    }
    if (motion & InputData::InputJoyDirectionUp && !joymotion_up) {
        joymotion_up = true;
        set_button_focus_up();
    } else {
        joymotion_up = false;
    }

    return false;
}

bool ButtonNavigator::static_window_joybutton_down(GuiWindow *sender, void *data, int button) {
    return (reinterpret_cast<ButtonNavigator *>(data))->window_joybutton_down(button);
}

bool ButtonNavigator::window_joybutton_down(int button) {
    binding.extract_from_config(config);
    if (binding.down.device == MappedKey::DeviceJoyButton && binding.down.param == button) {
        set_button_focus_down();
        return true;
    }
    if (binding.up.device == MappedKey::DeviceJoyButton && binding.up.param == button) {
        set_button_focus_up();
        return true;
    }
    if (binding.fire.device == MappedKey::DeviceJoyButton && binding.fire.param == button) {
        do_current_button_click();
        return true;
    }
    if (binding.escape.device == MappedKey::DeviceJoyButton && binding.escape.param == button) {
        if (on_cancel) {
            on_cancel(on_cancel_data);
        }
        return true;
    }

    return false;
}

void ButtonNavigator::set_button_focus_up() {
    current_index--;
    if (current_index < 0) {
        current_index = static_cast<int>(navigator_buttons.size()) - 1;
    }
    set_button_focus();
}

void ButtonNavigator::set_button_focus_down() {
    current_index++;
    if (current_index > static_cast<int>(navigator_buttons.size()) - 1) {
        current_index = 0;
    }
    set_button_focus();
}

void ButtonNavigator::do_current_button_click() {
    if (current_index >= 0 && current_index < static_cast<int>(navigator_buttons.size())) {
        GuiButton *btn = navigator_buttons[current_index];
        GuiButton::OnClick on_click = btn->get_on_click_func();
        if (on_click) {
            on_click(btn, btn->get_on_click_data());
        }
    }
}