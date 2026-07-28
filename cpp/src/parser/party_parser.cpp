#include "parser/party_parser.h"

namespace unboundmp::parser {

PartyParser::PartyParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : memory_(memory), addresses_(addresses) {}

ParseResult<RawPartyData> PartyParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::PartyReader reader(memory_, addresses_);
    auto result = reader.Read();
    if (result.ok()) {
        RawPartyData party_data;
        party_data.count = result.value->size();
        for (const auto& slot : result.value.value()) {
            party_data.slots.push_back(slot.bytes);
            
            ParsedPokemonData parsed{};
            if (slot.bytes.size() >= 100) {
                // Parse HP (unencrypted)
                std::memcpy(&parsed.current_hp, slot.bytes.data() + 86, 2);
                std::memcpy(&parsed.max_hp, slot.bytes.data() + 88, 2);
                
                // Parse Species (encrypted)
                uint32_t pid = 0;
                uint32_t otid = 0;
                std::memcpy(&pid, slot.bytes.data(), 4);
                std::memcpy(&otid, slot.bytes.data() + 4, 4);
                
                uint32_t key = pid ^ otid;
                int order = pid % 24;
                
                const int SUBSTRUCT_ORDER[24][4] = {
                    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
                    {0, 3, 1, 2}, {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2},
                    {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
                    {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0},
                    {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 0, 2, 1},
                    {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
                };
                
                int growth_index = -1;
                for (int i = 0; i < 4; i++) {
                    if (SUBSTRUCT_ORDER[order][i] == 0) {
                        growth_index = i;
                        break;
                    }
                }
                
                if (growth_index != -1) {
                    uint32_t growth_word0 = 0;
                    std::memcpy(&growth_word0, slot.bytes.data() + 32 + (growth_index * 12), 4);
                    uint32_t decrypted = growth_word0 ^ key;
                    parsed.species_id = decrypted & 0xFFFF;
                }
            }
            party_data.parsed_slots.push_back(parsed);
        }
        cached_result_ = ParseResult<RawPartyData>::Success(party_data, frame_count);
    } else {
        cached_result_ = ParseResult<RawPartyData>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
