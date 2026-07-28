import re

def update_db():
    path = 'd:/Unbound/pokemon/cpp/server/database/database.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    pattern = r"money BIGINT NOT NULL DEFAULT 0\s*\);"
    replacement = "money BIGINT NOT NULL DEFAULT 0,\n          save_state_blob BYTEA\n        );"

    new_content = re.sub(pattern, replacement, content)
    
    if new_content != content:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print('Updated database.cpp')
    else:
        print('Failed to update database.cpp schema')

update_db()
