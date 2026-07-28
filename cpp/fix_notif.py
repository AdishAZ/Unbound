def fix_notif():
    path = 'd:/Unbound/pokemon/cpp/src/ui/notification_center.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('int x = 1280 - width - 20;', 'int x = ctx.screen_width - width - 20;')
    content = content.replace('SDL_Rect overlay{0, 0, 1280, 720};', 'SDL_Rect overlay{0, 0, ctx.screen_width, ctx.screen_height};')
    content = content.replace('int x = (1280 - width) / 2;', 'int x = (ctx.screen_width - width) / 2;')
    content = content.replace('int y = (720 - height) / 2;', 'int y = (ctx.screen_height - height) / 2;')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_notif()
print("notification_center.cpp updated")
