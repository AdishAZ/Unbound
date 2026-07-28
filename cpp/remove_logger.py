def remove_logger():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widget.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('#include "utils/logger.h"', '')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

remove_logger()
print("logger removed from widget.cpp")
