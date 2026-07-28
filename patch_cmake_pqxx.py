with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'r') as f:
    content = f.read()

content = content.replace('add_executable(pqxx_test ../pqxx_test.cpp)\ntarget_link_libraries(pqxx_test PRIVATE pqxx::pqxx unboundmp_core)', '')

with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'w') as f:
    f.write(content)
