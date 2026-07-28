#include "ui/screen.h"
#include <algorithm>

namespace unboundmp::ui {

UIScreen::UIScreen(const std::string& name) : name_(name) {}

void UIScreen::FocusNext() {
    if (widgets_.empty()) return;
    int start_index = focused_widget_index_ >= 0 ? focused_widget_index_ : 0;
    for (size_t i = 1; i <= widgets_.size(); ++i) {
        int next_index = (start_index + i) % widgets_.size();
        if (widgets_[next_index]->IsVisible()) { // Simplified focus check
            FocusWidget(next_index);
            return;
        }
    }
}

void UIScreen::FocusPrev() {
    if (widgets_.empty()) return;
    int start_index = focused_widget_index_ >= 0 ? focused_widget_index_ : 0;
    for (size_t i = 1; i <= widgets_.size(); ++i) {
        int prev_index = (start_index - i + widgets_.size()) % widgets_.size();
        if (widgets_[prev_index]->IsVisible()) {
            FocusWidget(prev_index);
            return;
        }
    }
}

void UIScreen::FocusWidget(int index) {
    if (index >= 0 && index < static_cast<int>(widgets_.size())) {
        focused_widget_index_ = index;
        // Would typically call Focus() on the widget itself here, if the Widget class supports it
    }
}

} // namespace unboundmp::ui
