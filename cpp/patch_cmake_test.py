with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'r') as f:
    content = f.read()

target = '''
add_executable(character_system_test examples/character_system_test.cpp)
target_link_libraries(character_system_test PRIVATE
    unboundmp_core
    unboundmp_network
    unboundmp_server
)
'''
content += target

with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'w') as f:
    f.write(content)
