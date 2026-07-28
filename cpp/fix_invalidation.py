def fix_invalidation():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/settings_screen.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'std::shared_ptr<Widget> root_layout_;' in content_h:
        content_h = content_h.replace('std::shared_ptr<Widget> root_layout_;', 'std::shared_ptr<Container> root_layout_;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    # also check login_screen.h as it might have the same problem
    path_ls = 'd:/Unbound/pokemon/cpp/include/ui/screens/login_screen.h'
    with open(path_ls, 'r', encoding='utf-8') as f:
        content_ls = f.read()
    
    if 'std::shared_ptr<Widget> root_layout_;' in content_ls:
        content_ls = content_ls.replace('std::shared_ptr<Widget> root_layout_;', 'std::shared_ptr<Container> root_layout_;')
        with open(path_ls, 'w', encoding='utf-8') as f:
            f.write(content_ls)

    print('Fixed InvalidateLayout type issue')

fix_invalidation()
