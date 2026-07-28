def fix_errors():
    import glob
    
    for file in glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp'):
        with open(file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        modified = False
        if 'SetTexture(tex, true)' in content:
            content = content.replace('SetTexture(tex, true)', 'SetTexture(tex)')
            modified = True
            
        if 'SetBgColor' in content:
            content = content.replace('SetBgColor', 'SetBackgroundColor')
            modified = True
            
        if modified:
            with open(file, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f'Fixed {file}')

fix_errors()
