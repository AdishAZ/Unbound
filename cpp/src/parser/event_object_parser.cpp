#include "parser/event_object_parser.h"
#include "parser/coordinate_normalizer.h"

namespace unboundmp::parser {

// Memory addresses (same as EntityRenderer used)
static constexpr uint32_t OBJ_EVENTS_BASE = 0x02036E38;
static constexpr int OBJ_EVENT_SIZE = 0x24;
static constexpr int OBJ_EVENT_COUNT = 16;

EventObjectParser::EventObjectParser(const memory::MemoryApi& memory)
    : memory_(memory) {}

ParseResult<RawEventObjectData> EventObjectParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    RawEventObjectData data;
    data.events.reserve(OBJ_EVENT_COUNT);

    for (int i = 0; i < OBJ_EVENT_COUNT; ++i) {
        uint32_t oe = OBJ_EVENTS_BASE + i * OBJ_EVENT_SIZE;
        uint8_t flags0 = memory_.ReadU8(oe);
        if (!(flags0 & 0x01)) continue; // Not active

        uint8_t flags1 = memory_.ReadU8(oe + 0x01);
        if (flags1 & 0x10) continue; // Invisible

        int16_t raw_wx = static_cast<int16_t>(memory_.ReadU16(oe + 0x10));
        int16_t raw_wy = static_cast<int16_t>(memory_.ReadU16(oe + 0x12));
        
        int16_t wx = raw_wx;
        int16_t wy = raw_wy;
        // NORMALIZE COORDINATES: Strip the hardware 7-tile padding
        CoordinateNormalizer::Normalize(wx, wy);

        bool is_player = (flags1 & 0x80) != 0;
        
        uint8_t sprite_id = memory_.ReadU8(oe + 0x04);
        uint8_t graphics_id = memory_.ReadU8(oe + 0x05);
        uint8_t direction = memory_.ReadU8(oe + 0x18); // Actually it's just facing dir? Let's keep it 0 for now if unknown.

        // Log every 60 frames to avoid console spam
        if (frame_count % 60 == 0) {
            printf("[Pipeline] EventObjectParser - ID: %d | Type: %s | Raw: (%d, %d) -> Norm: (%d, %d) | GfxID: %d | Active: 1\n",
                   i, is_player ? "Player" : "NPC", raw_wx, raw_wy, wx, wy, graphics_id);
        }

        EventObjectSnapshot snap;
        snap.id = i;
        snap.x = wx;
        snap.y = wy;
        snap.is_player = is_player;
        snap.sprite_id = sprite_id;
        snap.graphics_id = graphics_id;
        snap.direction = direction;
        
        data.events.push_back(snap);
    }

    cached_result_ = ParseResult<RawEventObjectData>::Success(data, frame_count);
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
