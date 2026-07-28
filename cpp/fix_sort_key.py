import os
import glob

files = glob.glob('d:/Unbound/pokemon/cpp/src/render/*.cpp')
for file in files:
    if "render_queue" in file:
        continue
    with open(file, 'r') as f:
        content = f.read()
    
    new_content = content.replace('.z_index', '.sort_key.layer').replace('.sort_y', '.sort_key.sort_y')
    
    if new_content != content:
        with open(file, 'w') as f:
            f.write(new_content)
        print(f"Updated {file}")
