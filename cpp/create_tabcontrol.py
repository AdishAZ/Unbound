def create_tabcontrol():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'class TabControl' not in content_h:
        tab_h = '''
class TabControl : public Container {
public:
    explicit TabControl(const std::string& id = "");
    void AddTab(const std::string& title, std::shared_ptr<Widget> content);
    void SelectTab(int index);
    
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;

private:
    std::shared_ptr<HorizontalLayout> tab_bar_;
    std::shared_ptr<Container> content_area_;
    int active_tab_ = -1;
    std::vector<std::shared_ptr<Widget>> tab_contents_;
};
'''
        content_h = content_h.replace('class ScrollView', tab_h + '\nclass ScrollView')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if 'TabControl::TabControl' not in content_cpp:
        tab_cpp = '''
TabControl::TabControl(const std::string& id) : Container(id) {
    tab_bar_ = std::make_shared<HorizontalLayout>(id + "_bar");
    tab_bar_->SetHeightPolicy(SizePolicy::Fixed);
    tab_bar_->SetBounds({0, 0, 0, 30});
    
    content_area_ = std::make_shared<Panel>(id + "_content");
    content_area_->SetWidthPolicy(SizePolicy::Expand);
    content_area_->SetHeightPolicy(SizePolicy::Expand);
    
    AddChild(tab_bar_);
    AddChild(content_area_);
}

void TabControl::AddTab(const std::string& title, std::shared_ptr<Widget> content) {
    int index = (int)tab_contents_.size();
    
    auto btn = std::make_shared<Button>(id_ + "_btn_" + std::to_string(index));
    btn->SetText(title);
    btn->SetWidthPolicy(SizePolicy::Fixed);
    btn->SetBounds({0, 0, 100, 30});
    
    btn->OnClick([this, index]() {
        SelectTab(index);
    });
    
    tab_bar_->AddChild(btn);
    content_area_->AddChild(content);
    tab_contents_.push_back(content);
    
    if (active_tab_ == -1) {
        SelectTab(0);
    } else {
        content->SetVisible(false);
    }
}

void TabControl::SelectTab(int index) {
    if (index < 0 || index >= (int)tab_contents_.size()) return;
    active_tab_ = index;
    for (int i = 0; i < (int)tab_contents_.size(); ++i) {
        tab_contents_[i]->SetVisible(i == index);
    }
    InvalidateLayout();
}

void TabControl::Update(float dt) {
    Container::Update(dt);
}

void TabControl::PerformLayout() {
    int w = bounds_.width;
    int h = bounds_.height;
    
    tab_bar_->SetBounds({bounds_.x, bounds_.y, w, 30});
    content_area_->SetBounds({bounds_.x, bounds_.y + 30, w, h - 30});
    
    // Size contents to fill content area
    for (auto& content : tab_contents_) {
        content->SetBounds({bounds_.x, bounds_.y + 30, w, h - 30});
    }
}

Size TabControl::GetPreferredSize() const {
    int max_w = 0;
    int max_h = 30; // bar height
    for (auto& content : tab_contents_) {
        Size ps = content->GetPreferredSize();
        max_w = std::max(max_w, ps.width);
        max_h = std::max(max_h, 30 + ps.height);
    }
    return {max_w, max_h};
}
'''
        idx = content_cpp.find('// GridLayout')
        if idx == -1: idx = content_cpp.find('void GridLayout::')
        content_cpp = content_cpp[:idx] + tab_cpp + content_cpp[idx:]
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)
        print('TabControl created')

create_tabcontrol()
