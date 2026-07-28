def add_layouts():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'class VerticalLayout' not in content:
        old_container = '''class Container : public Widget {
public:
    explicit Container(const std::string& id = "") : Widget(id) {}
    void SetDirection(LayoutDirection dir) { direction_ = dir; }
    void SetSpacing(int spacing) { spacing_ = spacing; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    LayoutDirection direction_ = LayoutDirection::Vertical;
    int spacing_ = 0;
};'''
        
        new_container = '''class Container : public Widget {
public:
    explicit Container(const std::string& id = "") : Widget(id) {}
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;
    virtual void PerformLayout() {} // Override for layout
    
    // Call when children change or resize
    void InvalidateLayout() { layout_dirty_ = true; }

protected:
    bool layout_dirty_ = true;
};

class VerticalLayout : public Container {
public:
    explicit VerticalLayout(const std::string& id = "") : Container(id) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
private:
    int spacing_ = 0;
};

class HorizontalLayout : public Container {
public:
    explicit HorizontalLayout(const std::string& id = "") : Container(id) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
private:
    int spacing_ = 0;
};

class GridLayout : public Container {
public:
    explicit GridLayout(int cols, const std::string& id = "") : Container(id), columns_(cols) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void SetColumns(int c) { columns_ = c; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
private:
    int columns_ = 1;
    int spacing_ = 0;
};

class AnchorLayout : public Container {
public:
    explicit AnchorLayout(const std::string& id = "") : Container(id) {}
    void Update(float dt) override;
    void PerformLayout() override;
};'''

        content = content.replace(old_container, new_container)
        
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

add_layouts()
print("widgets.h updated with Layouts")
