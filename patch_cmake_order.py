with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'r') as f:
    content = f.read()

content = content.replace('set_target_properties(unboundmp_server PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE")', '')

content = content.replace('''target_link_libraries(unboundmp_server
        PRIVATE
            unboundmp_network
            unboundmp_protocol
            unboundmp_persistence
            libpqxx::pqxx
            unofficial::argon2::libargon2
            Threads::Threads
    )''', '''target_link_libraries(unboundmp_server
        PRIVATE
            libpqxx::pqxx
            unboundmp_network
            unboundmp_protocol
            unboundmp_persistence
            unofficial::argon2::libargon2
            Threads::Threads
    )''')

with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'w') as f:
    f.write(content)
