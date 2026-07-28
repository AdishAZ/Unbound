import requests
import os
import shutil

def main():
    print("Fetching Unbound species list...")
    resp = requests.get('https://raw.githubusercontent.com/Zannael/PUSE/master/backend/data/pokemon.txt')
    unbound_lines = resp.text.splitlines()
    
    unbound_map = {}
    for line in unbound_lines:
        if ':' in line:
            parts = line.split(':')
            uid = int(parts[0])
            name = parts[1].strip().lower().replace('?', '')
            # Handle some naming quirks
            if name == 'mr. mime': name = 'mr-mime'
            elif name == 'mime jr': name = 'mime-jr'
            elif name == 'nidoran(f)' or name == 'nidoran?': name = 'nidoran-f'
            elif name == 'nidoran(m)' or name == 'nidoran?': name = 'nidoran-m'
            unbound_map[name] = uid

    print("Fetching National Dex list...")
    resp = requests.get('https://pokeapi.co/api/v2/pokemon?limit=1500')
    national_data = resp.json()['results']
    
    national_map = {}
    for entry in national_data:
        name = entry['name']
        url = entry['url']
        nid = int(url.rstrip('/').split('/')[-1])
        national_map[name] = nid
        
    # Manual overrides for some names
    national_map['nidoranf'] = national_map.get('nidoran-f', 29)
    national_map['nidoranm'] = national_map.get('nidoran-m', 32)
    national_map['ho-oh'] = 250
    national_map['mr. mime'] = 122
    national_map['mime jr.'] = 439
    
    source_dir = 'assets/pokemon'
    dest_dir = 'assets/unbound_pokemon'
    
    os.makedirs(dest_dir, exist_ok=True)
    
    copied = 0
    missing = []
    
    for name, uid in unbound_map.items():
        if uid == 0: continue
        
        nid = None
        # Try exact match
        if name in national_map:
            nid = national_map[name]
        else:
            # Try removing special chars
            clean_name = name.replace('-', '').replace(' ', '').replace('.', '')
            for n_name, n_id in national_map.items():
                c_name = n_name.replace('-', '').replace(' ', '').replace('.', '')
                if clean_name == c_name:
                    nid = n_id
                    break
        
        if nid is not None:
            src = os.path.join(source_dir, f"{nid}.bmp")
            dst = os.path.join(dest_dir, f"{uid}.bmp")
            if os.path.exists(src):
                shutil.copy2(src, dst)
                copied += 1
            else:
                missing.append(f"{name} (NID {nid} BMP not found)")
        else:
            missing.append(f"{name} (Not found in National Dex)")
            
    print(f"Successfully mapped {copied} sprites.")
    if missing[:10]:
        print("Some missing mappings (first 10):")
        for m in missing[:10]:
            print(" -", m)

if __name__ == "__main__":
    main()
