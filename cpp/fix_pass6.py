import glob
import re

def fix_checkboxes(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    content = content.replace('->SetLabel', '->SetText')
    
    checkbox_vars = set(re.findall(r'(\w+)\s*=\s*std::make_shared<Checkbox>', content))
    for var in checkbox_vars:
        content = content.replace(f'{var}->SetText', f'{var}->SetLabel')
        
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

for f in glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp'):
    fix_checkboxes(f)

