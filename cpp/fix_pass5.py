import re

def fix_settings():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # We will change all ->SetLabel to ->SetText first.
    content = content.replace('->SetLabel', '->SetText')
    
    # Now look for variables assigned to make_shared<Checkbox>
    checkbox_vars = set(re.findall(r'(\w+)\s*=\s*std::make_shared<Checkbox>', content))
    
    for var in checkbox_vars:
        content = content.replace(f'{var}->SetText', f'{var}->SetLabel')
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print("Fixed settings")

def fix_ui_types():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_types.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'float alpha = 1.0f;' not in content:
        content = content.replace('int screen_height = 0;', 'int screen_height = 0;\n    float alpha = 1.0f;')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
    print("Fixed ui_types")

fix_settings()
fix_ui_types()

