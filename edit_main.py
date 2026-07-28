def edit_main():
    path = 'd:/Unbound/pokemon/cpp/client/main.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add log_manager header
    if '#include "core/log_manager.h"' not in content:
        content = content.replace('#include "ui/ui_engine.h"', '#include "ui/ui_engine.h"\n#include "core/log_manager.h"')

    # Add log manager initialize
    if 'LogManager::Get().Initialize' not in content:
        content = content.replace('unboundmp::ui::UIEngine ui_engine;', 'unboundmp::core::LogManager::Get().Initialize("logs", 5 * 1024 * 1024, 3);\n    unboundmp::ui::UIEngine ui_engine;')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_main()
print("main.cpp updated")
