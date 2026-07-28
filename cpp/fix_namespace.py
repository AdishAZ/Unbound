def fix_namespace():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the last occurrence of '} // namespace unboundmp::ui'
    ns_idx = content.rfind('} // namespace unboundmp::ui')
    if ns_idx != -1:
        # Move it to the end
        content = content[:ns_idx] + content[ns_idx + len('} // namespace unboundmp::ui'):] + '\n} // namespace unboundmp::ui\n'
        
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

fix_namespace()
print("widgets.cpp namespace fixed")
