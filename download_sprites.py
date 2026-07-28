import os
import requests
from io import BytesIO
try:
    from PIL import Image
except ImportError:
    import subprocess
    import sys
    subprocess.check_call([sys.executable, "-m", "pip", "install", "Pillow"])
    from PIL import Image

def download_sprites(limit=400):
    base_url = "https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/{}.png"
    out_dir = "assets/pokemon"
    os.makedirs(out_dir, exist_ok=True)
    
    print(f"Downloading up to {limit} pokemon sprites...")
    for i in range(1, limit + 1):
        out_path = os.path.join(out_dir, f"{i}.bmp")
        if os.path.exists(out_path):
            continue
            
        url = base_url.format(i)
        try:
            resp = requests.get(url, timeout=5)
            if resp.status_code == 200:
                img = Image.open(BytesIO(resp.content))
                # Convert to RGB to save as BMP (handling transparency if necessary)
                # For BMP, we can use RGBA if supported, but typically RGB with a colorkey is used.
                # Let's create a background with magenta (commonly used for colorkey) 
                # or just save as RGBA and hope SDL_LoadBMP supports it.
                # Actually, standard SDL_LoadBMP doesn't always support alpha well. 
                # Let's convert to RGB with a solid background color (e.g., magenta 255,0,255).
                background = Image.new("RGB", img.size, (255, 0, 255))
                if img.mode in ('RGBA', 'LA') or (img.mode == 'P' and 'transparency' in img.info):
                    background.paste(img, mask=img.convert('RGBA').split()[3])
                else:
                    background = img.convert("RGB")
                
                out_path = os.path.join(out_dir, f"{i}.bmp")
                background.save(out_path, "BMP")
                if i % 50 == 0:
                    print(f"Downloaded {i}/{limit}...")
        except Exception as e:
            print(f"Failed to download sprite {i}: {e}")
            
if __name__ == "__main__":
    download_sprites(900)
    print("Done downloading sprites.")
