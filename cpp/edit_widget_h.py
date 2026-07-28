def edit_widget_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widget.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_decl = '''    public:
    // Helper: draw text as monospace grid (8x14 per char)
    static void DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, int char_width = 8, int char_height = 14);
    // Helper: measure text width
    static int MeasureTextWidth(const std::string& text, int char_width = 8);'''

    new_decl = '''public:
    static void DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, const std::string& font = "default", int size = 16, Alignment align = Alignment::TopLeft);
    static void DrawTextWrapped(const RenderContext& ctx, const std::string& text, int x, int y, int wrap_width, const Color& color, const std::string& font = "default", int size = 16, Alignment align = Alignment::TopLeft);
    static int MeasureTextWidth(const RenderContext& ctx, const std::string& text, const std::string& font = "default", int size = 16);
    static void MeasureText(const RenderContext& ctx, const std::string& text, int& w, int& h, const std::string& font = "default", int size = 16);'''

    content = content.replace(old_decl, new_decl)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_widget_h()
print("widget.h modified")
