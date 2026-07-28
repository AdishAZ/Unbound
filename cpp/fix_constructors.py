import glob
import re

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    for old, new in replacements:
        if hasattr(old, 'sub'):
            content = old.sub(new, content)
        else:
            content = content.replace(old, new)
            
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

screens_cpp = glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp')
for f in screens_cpp:
    screen_name = f.replace('\\', '/').split('/')[-1].replace('.cpp', '')
    class_name = ''.join([w.capitalize() for w in screen_name.split('_')])
    
    fix_file(f, [
        (re.compile(fr'{class_name}::{class_name}\((.*?)\)\s*:'), fr'{class_name}::{class_name}(\1) : UIScreen("{class_name}"),'),
        (re.compile(fr'{class_name}::{class_name}\((.*?)\)\s*{{'), fr'{class_name}::{class_name}(\1) : UIScreen("{class_name}") {{')
    ])
