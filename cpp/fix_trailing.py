import glob
import re

def fix_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    content = re.sub(r'\s*:\s*UIScreen\("[a-zA-Z]+"\'?\)\s*\{', ' {', content)
    content = re.sub(r'\s*:\s*UIScreen\("[a-zA-Z]+"\)\s*\{', ' {', content)
    
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

screens_cpp = glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp')
for f in screens_cpp:
    fix_file(f)
