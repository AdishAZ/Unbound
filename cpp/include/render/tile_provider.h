#pragma once
#include <cstdint>

namespace unboundmp::render {

class TileProvider {
public:
    virtual ~TileProvider() = default;
    
    // Returns the metatile ID, or -1 if out of bounds. 
    // out_tiles[4] will contain the four 8x8 tile IDs (top-left, top-right, bottom-left, bottom-right).
    virtual int GetMetatileAt(int x, int y, int layer, int out_tiles[4]) = 0;
    
    virtual class TextureAtlas* GetTextureAtlas() { return nullptr; }
    virtual void UpdateMap(uint8_t map_bank, uint8_t map_num) {}
    
    virtual int GetMapWidth() const { return 0; }
    virtual int GetMapHeight() const { return 0; }
    virtual int GetBorderWidth() const { return 2; }  // Gen III standard
    virtual int GetBorderHeight() const { return 2; } // Gen III standard
    
    virtual uint32_t GetVersion() const { return 0; }
};

}
