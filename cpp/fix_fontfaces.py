def fix_fontfaces():
    path = 'd:/Unbound/pokemon/cpp/src/ui/font_manager.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('TTF_GetFontFaces(font)', 'TTF_FontFaces(font)')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_fontfaces()
print("font_manager.cpp fixed")
