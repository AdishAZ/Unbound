// Standalone smoke test for the memory abstraction layer. Uses a fake
// IEmulatorCore backed by a plain byte array instead of a real mgba core,
// so this proves the AddressTable -> MemoryApi -> reader pipeline works
// without needing a ROM (or libmgba) at all. Not a real client - see
// mgba_emulator_core.h for the real backend, built only when
// UNBOUNDMP_WITH_MGBA is enabled.
#include <cstring>
#include <fstream>
#include <iostream>

#include "emulator/emulator_core.h"
#include "memory/address_table.h"
#include "memory/direction_reader.h"
#include "memory/map_reader.h"
#include "memory/memory_api.h"
#include "memory/movement_state_reader.h"
#include "memory/position_reader.h"
#include "memory/sprite_reader.h"

namespace {

using namespace unboundmp;

// Minimal fake core: a flat byte array covering just enough of the EWRAM
// address range to exercise the readers above. Not a real emulator - just
// enough to prove MemoryApi/readers correctly turn AddressTable entries
// into typed values.
class FakeEmulatorCore final : public emulator::IEmulatorCore {
 public:
  explicit FakeEmulatorCore(size_t ewram_bytes) : ewram_(ewram_bytes, 0) {}
  const void* GetVideoBuffer() const override
  {
    return nullptr;
}

  size_t GetVideoPitch() const override
{
    return 0;
}

  emulator::EmulatorResult Initialize() override { return emulator::EmulatorResult::Success(); }
  emulator::EmulatorResult LoadRom(const std::string&) override {
    return emulator::EmulatorResult::Success();
  }
  emulator::EmulatorResult LoadSave(const std::string&) override {
    return emulator::EmulatorResult::Success();
  }
  emulator::EmulatorResult Start() override { return emulator::EmulatorResult::Success(); }
  emulator::EmulatorResult Pause() override { return emulator::EmulatorResult::Success(); }
  emulator::EmulatorResult Resume() override { return emulator::EmulatorResult::Success(); }
  emulator::EmulatorResult Reset() override { return emulator::EmulatorResult::Success(); }
  emulator::EmulatorResult Stop() override { return emulator::EmulatorResult::Success(); }
  void Shutdown() override {}
  emulator::EmulatorState State() const override { return emulator::EmulatorState::kRunning; }
  void SetInput(const emulator::InputState&) override {}

  uint8_t ReadU8(uint32_t address) override { return Backing(address).at(Offset(address)); }
  uint16_t ReadU16(uint32_t address) override {
    uint16_t value;
    std::memcpy(&value, &Backing(address).at(Offset(address)), sizeof(value));
    return value;
  }
  uint32_t ReadU32(uint32_t address) override {
    uint32_t value;
    std::memcpy(&value, &Backing(address).at(Offset(address)), sizeof(value));
    return value;
  }
  
  void WriteU8(uint32_t address, uint8_t value) override {
      Backing(address).at(Offset(address)) = value;
  }
  void WriteU16(uint32_t address, uint16_t value) override {
      std::memcpy(&Backing(address).at(Offset(address)), &value, sizeof(value));
  }
  void WriteU32(uint32_t address, uint32_t value) override {
      std::memcpy(&Backing(address).at(Offset(address)), &value, sizeof(value));
  }
  void WriteBytes(uint32_t address, const uint8_t* data, size_t length) override {
      if (length == 0 || !data) return;
      std::memcpy(&Backing(address).at(Offset(address)), data, length);
  }
  std::optional<std::string> GameTitle() const override { return "FAKE UNBOUND"; }
  std::optional<std::string> GameCode() const override { return "FAKE"; }

  // --- Stage 3 Stubs ---
  bool IsRunning() const override { return true; }
  bool IsPaused() const override { return false; }
  bool IsLoaded() const override { return true; }
  uint32_t GetFrameCount() const override { return 0; }
  float GetFPS() const override { return 60.0f; }
  float GetCurrentSpeed() const override { return 1.0f; }
  uint32_t RomCrc32() const override { return 0; }
  size_t RomSize() const override { return 0; }
  std::optional<std::string> SaveType() const override { return std::nullopt; }
  bool HasSaveState(int) const override { return false; }
  emulator::EmulatorResult DeleteSaveState(int) override { return emulator::EmulatorResult::Success(); }
  std::vector<int> ListSaveStates() const override { return {}; }
  std::optional<emulator::SaveStateMetadata> GetSaveStateMetadata(int) const override { return std::nullopt; }
  size_t RegisterEventCallback(emulator::EventCallback) override { return 0; }
  void UnregisterEventCallback(size_t) override {}

  // Test helper - writes directly into the fake EWRAM to set up scenarios.
  void PokeU8(uint32_t address, uint8_t value) { Backing(address).at(Offset(address)) = value; }

 private:
  // The fake core only needs to answer for the two GBA memory regions this
  // example actually touches: EWRAM (player_x/y/facing/etc., addressed via
  // AddressTable symbols) and OAM (fixed hardware sprite attributes, read
  // by SpriteReader with no AddressTable involved at all). Routing by
  // address range - rather than one array offset by a single fixed base -
  // is what a real backend does too (see mgba_emulator_core.h): different
  // regions of the 32-bit CPU address space are genuinely different
  // backing memory, not one contiguous block.
  std::vector<uint8_t>& Backing(uint32_t address) {
    if (address >= memory::gba::kOamBase && address < memory::gba::kOamBase + memory::gba::kOamSize) {
      return oam_;
    }
    return ewram_;
  }
  size_t Offset(uint32_t address) const {
    if (address >= memory::gba::kOamBase && address < memory::gba::kOamBase + memory::gba::kOamSize) {
      return static_cast<size_t>(address - memory::gba::kOamBase);
    }
    return static_cast<size_t>(address - memory::gba::kEwramBase);
  }
  std::vector<uint8_t> ewram_;
  std::vector<uint8_t> oam_ = std::vector<uint8_t>(memory::gba::kOamSize, 0);
};

}  // namespace

int main() {
  using namespace unboundmp;

  FakeEmulatorCore core(0x1000);
  memory::MemoryApi memory_api(core);

  // Simulate having reverse-engineered addresses by writing a small
  // AddressTable config to a temp file, then loading it exactly the way
  // a real deployment would (see docs/REVERSE_ENGINEERING.md).
  const std::string config_path = "memory_probe_example.cfg";
  {
    std::ofstream out(config_path);
    out << "# fake addresses for the smoke test - NOT real Unbound addresses\n";
    out << "player_map_bank = 0x02000000, u8\n";
    out << "player_map_number = 0x02000001, u8\n";
    out << "player_x = 0x02000002, u16\n";
    out << "player_y = 0x02000004, u16\n";
    out << "player_facing = 0x02000006, u8\n";
    out << "player_movement_flags = 0x02000007, u8\n";
  }

  std::string load_error;
  auto table_opt = memory::AddressTable::LoadFromFile(config_path, load_error);
  if (!table_opt) {
    std::cerr << "FAILED to load address table: " << load_error << "\n";
    return 1;
  }
  memory::AddressTable table = std::move(*table_opt);
  std::cout << "Loaded " << table.ConfiguredCount() << " symbols\n";

  core.PokeU8(0x02000000, 3);   // map bank
  core.PokeU8(0x02000001, 12);  // map number
  core.PokeU8(0x02000002, 42);  // x low byte
  core.PokeU8(0x02000004, 7);   // y low byte
  core.PokeU8(0x02000006, 2);   // facing = DIR_NORTH per the decomp hypothesis
  core.PokeU8(0x02000007, 0b00001000);  // surfing bit set

  memory::MapReader map_reader(memory_api, table);
  auto map_result = map_reader.Read();
  if (!map_result) {
    std::cerr << "map read failed: " << map_result.error << "\n";
    return 1;
  }
  std::cout << "Map: bank=" << map_result.value->bank << " number=" << map_result.value->number
            << "\n";

  memory::PositionReader position_reader(memory_api, table);
  auto position_result = position_reader.Read();
  std::cout << "Position: x=" << position_result.value->x << " y=" << position_result.value->y
            << "\n";

  memory::DirectionReader direction_reader(memory_api, table,
                                            memory::DirectionEncoding::PokeemeraldDecompHypothesis());
  auto direction_result = direction_reader.Read();
  const bool facing_north = direction_result && *direction_result.value == memory::FacingDirection::kNorth;
  std::cout << "Facing north (expected true): " << (facing_north ? "true" : "false") << "\n";

  memory::MovementStateReader movement_reader(memory_api, table,
                                               memory::MovementBitLayout::PokeemeraldDecompHypothesis());
  auto movement_result = movement_reader.Read();
  std::cout << "Surfing (expected true): " << (movement_result.value->surfing ? "true" : "false")
            << "\n";

  // Symbol deliberately left unconfigured - proves the "fail loudly, don't
  // guess" contract.
  memory::PositionReader unconfigured(memory_api, memory::AddressTable{});
  auto unconfigured_result = unconfigured.Read();
  std::cout << "Unconfigured read ok (expected false): " << unconfigured_result.ok() << "\n";
  std::cout << "Unconfigured read error: " << unconfigured_result.error << "\n";

  // SpriteReader needs no address table at all - fixed hardware OAM.
  memory::SpriteReader sprite_reader(memory_api);
  auto sprite = sprite_reader.Read(0);
  std::cout << "OAM slot 0 read without any RE'd addresses: "
            << (sprite.has_value() ? "ok" : "FAILED") << "\n";

  const bool all_ok = map_result.ok() && position_result.ok() && direction_result.ok() &&
                       movement_result.ok() && facing_north && movement_result.value->surfing &&
                       !unconfigured_result.ok() && sprite.has_value();

  std::cout << (all_ok ? "MEMORY PROBE EXAMPLE: PASS\n" : "MEMORY PROBE EXAMPLE: FAIL\n");
  return all_ok ? 0 : 1;
}
