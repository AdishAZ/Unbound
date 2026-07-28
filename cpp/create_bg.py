import struct
import math

def create_gradient_bmp(filename, width, height):
    # Dark blue/purple gradient
    def get_color(x, y):
        # radial gradient from top left (dark blue) to bottom right (dark purple)
        dx = x / width
        dy = y / height
        
        # Color 1: #0A1128 (Dark Blue)
        r1, g1, b1 = 10, 17, 40
        # Color 2: #1F0D3D (Dark Purple)
        r2, g2, b2 = 31, 13, 61
        
        t = math.sqrt(dx*dx + dy*dy) / 1.414
        r = int(r1 * (1-t) + r2 * t)
        g = int(g1 * (1-t) + g2 * t)
        b = int(b1 * (1-t) + b2 * t)
        
        return bytes([b, g, r]) # BMP is BGR

    # Padding for 4-byte alignment
    row_padded = (width * 3 + 3) & (~3)
    data = bytearray(row_padded * height)
    
    for y in range(height):
        # BMP is bottom-up, so invert y for drawing
        draw_y = height - 1 - y
        for x in range(width):
            idx = y * row_padded + x * 3
            data[idx:idx+3] = get_color(x, draw_y)

    filesize = 54 + len(data)
    
    with open(filename, 'wb') as f:
        # BMP Header
        f.write(b'BM')
        f.write(struct.pack('<I', filesize))
        f.write(b'\x00\x00\x00\x00')
        f.write(struct.pack('<I', 54))
        
        # DIB Header (BITMAPINFOHEADER)
        f.write(struct.pack('<I', 40)) # Header size
        f.write(struct.pack('<I', width))
        f.write(struct.pack('<I', height))
        f.write(struct.pack('<H', 1)) # Color planes
        f.write(struct.pack('<H', 24)) # Bits per pixel
        f.write(struct.pack('<I', 0)) # Compression
        f.write(struct.pack('<I', len(data))) # Image size
        f.write(struct.pack('<I', 2835)) # X pixels per meter
        f.write(struct.pack('<I', 2835)) # Y pixels per meter
        f.write(struct.pack('<I', 0)) # Colors in color table
        f.write(struct.pack('<I', 0)) # Important color count
        
        # Pixel data
        f.write(data)

import os
os.makedirs('d:/Unbound/pokemon/assets/ui', exist_ok=True)
create_gradient_bmp('d:/Unbound/pokemon/assets/ui/login_bg.bmp', 960, 640)
print('login_bg.bmp created')
