with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'r') as f:
    content = f.read()

content = content.replace('set_target_properties(unboundmp_server PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE")', '')

with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'w') as f:
    f.write(content)
