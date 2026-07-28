import os
import glob

replacements = {
    'kEmulator': 'kDebug',
    'kRemotePlayers': 'kEntities',
    'kNPCOverlay': 'kEntities',
    'kParticles': 'kWeather',
    'kEffects': 'kWeather',
    'RenderLayerZ::RenderLayerZ::': 'RenderLayerZ::'
}

files = glob.glob('d:/Unbound/pokemon/cpp/src/render/*.cpp')
for file in files:
    with open(file, 'r') as f:
        content = f.read()
    
    new_content = content
    for old, new in replacements.items():
        new_content = new_content.replace(old, new)
        
    if new_content != content:
        with open(file, 'w') as f:
            f.write(new_content)
        print(f"Updated {file}")
