#include "parser/local_player_parser.h"

namespace unboundmp::parser {

LocalPlayerParser::LocalPlayerParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : memory_(memory), addresses_(addresses) {
    position_parser_ = std::make_unique<PositionParser>(memory_, addresses_);
    map_parser_ = std::make_unique<MapParser>(memory_, addresses_);
    
    // Default encodings for direction and movement layouts based on memory readers requirements
    memory::DirectionEncoding dir_encoding;
    direction_parser_ = std::make_unique<DirectionParser>(memory_, addresses_, dir_encoding);
    
    memory::MovementBitLayout move_layout;
    movement_parser_ = std::make_unique<MovementParser>(memory_, addresses_, move_layout);
}

ParseResult<LocalPlayerSnapshot> LocalPlayerParser::Parse(int64_t frame_count) {
    LocalPlayerSnapshot snapshot;

    auto pos_result = position_parser_->Parse(frame_count);
    if (!pos_result.ok()) return ParseResult<LocalPlayerSnapshot>::Failure(pos_result.error);
    snapshot.position = pos_result.value.value();

    auto map_result = map_parser_->Parse(frame_count);
    if (!map_result.ok()) return ParseResult<LocalPlayerSnapshot>::Failure(map_result.error);
    snapshot.map = map_result.value.value();

    auto dir_result = direction_parser_->Parse(frame_count);
    if (!dir_result.ok()) return ParseResult<LocalPlayerSnapshot>::Failure(dir_result.error);
    snapshot.facing = dir_result.value.value();

    auto move_result = movement_parser_->Parse(frame_count);
    if (!move_result.ok()) return ParseResult<LocalPlayerSnapshot>::Failure(move_result.error);
    snapshot.movement = move_result.value.value();

    return ParseResult<LocalPlayerSnapshot>::Success(snapshot, frame_count);
}

} // namespace unboundmp::parser
