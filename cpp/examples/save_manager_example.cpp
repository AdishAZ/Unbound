// Standalone smoke test for the Milestone 12 save-synchronization layer.
// Uses a real temp directory and real file I/O (SaveManager operates on
// actual save bytes on disk, unlike the pure-logic gameplay/render/
// interaction layers) but no emulator, no ROM, and no network socket - it
// simulates the two shapes NetworkManager's LinkSessionEnd handler would
// eventually drive this with: a completed trade and a cancelled battle.
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "save/save_manager.h"

namespace fs = std::filesystem;

namespace {

void WriteFile(const std::string& path, const std::string& contents) {
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  f << contents;
}

std::string ReadFile(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

}  // namespace

int main() {
  using namespace unboundmp;

  const fs::path temp_dir = fs::temp_directory_path() / "unboundmp_save_manager_example";
  std::error_code ec;
  fs::remove_all(temp_dir, ec);
  fs::create_directories(temp_dir, ec);

  const std::string rom_path = (temp_dir / "pokemon_unbound.gba").string();
  const std::string save_path = (temp_dir / "pokemon_unbound.sav").string();

  // Pretend a save already exists from prior play (this file's bytes are
  // never inspected, interpreted, or written to except as opaque blobs -
  // matching the ROM/save-file contract every other layer in this project
  // follows).
  WriteFile(save_path, "SAVE_V1_PARTY:PIKACHU,CHARMANDER");

  save::SaveManager manager(/*max_backups_per_save=*/5);
  auto init_result = manager.InitializeForRom(rom_path);
  std::cout << "InitializeForRom: ok=" << init_result.ok << " path=" << manager.save_path()
            << "\n";
  assert(init_result.ok);
  assert(manager.save_path() == save_path);

  // A session_start backup should have been taken automatically since a
  // save already existed.
  auto backups_after_init = manager.ListBackups();
  std::cout << "Backups after init: " << backups_after_init.size() << " (expected 1)\n";
  assert(backups_after_init.size() == 1);
  assert(backups_after_init[0].tag == "session_start");

  // --- Scenario 1: a completed trade that actually writes the save -------
  constexpr uint32_t kTradeSessionId = 100;
  auto begin_trade = manager.BeginLinkSession(kTradeSessionId, save::LinkKind::kTrade);
  assert(begin_trade.ok);

  // Simulate the emulator's own link-cable emulation mutating the save as
  // a real trade would (SaveManager never does this itself - it only
  // observes the resulting bytes).
  WriteFile(save_path, "SAVE_V1_PARTY:RAICHU,CHARMANDER");

  auto end_trade = manager.EndLinkSession(kTradeSessionId, save::LinkKind::kTrade,
                                           /*completed=*/true);
  std::cout << "Trade completed: ok=" << end_trade.ok << " persisted=" << end_trade.persisted
            << " backup=" << (end_trade.backup ? end_trade.backup->path : "<none>") << "\n";
  assert(end_trade.ok);
  assert(end_trade.persisted);  // this is the milestone's core guarantee

  // --- Scenario 2: a cancelled battle - save should be untouched ---------
  constexpr uint32_t kBattleSessionId = 200;
  auto begin_battle = manager.BeginLinkSession(kBattleSessionId, save::LinkKind::kBattle);
  assert(begin_battle.ok);

  // No write happens - the battle was cancelled before completion.
  auto end_battle = manager.EndLinkSession(kBattleSessionId, save::LinkKind::kBattle,
                                            /*completed=*/false);
  std::cout << "Battle cancelled: ok=" << end_battle.ok << " persisted=" << end_battle.persisted
            << "\n";
  assert(end_battle.ok);
  assert(!end_battle.persisted);

  // --- Scenario 3: a completed battle that DIDN'T actually write --------
  // (e.g. the game hadn't flushed to the save file yet) - EndLinkSession
  // should flag this instead of silently reporting success.
  constexpr uint32_t kStuckBattleId = 300;
  manager.BeginLinkSession(kStuckBattleId, save::LinkKind::kBattle);
  auto end_stuck = manager.EndLinkSession(kStuckBattleId, save::LinkKind::kBattle,
                                           /*completed=*/true);
  std::cout << "Battle completed but unchanged: ok=" << end_stuck.ok
            << " persisted=" << end_stuck.persisted << " message=\"" << end_stuck.message
            << "\"\n";
  assert(!end_stuck.ok);
  assert(!end_stuck.persisted);

  // --- Conflict detection: an external process rewrites the save --------
  auto conflict_before = manager.CheckForExternalConflict();
  std::cout << "Conflict check (no external change): conflict=" << conflict_before.conflict
            << "\n";
  assert(!conflict_before.conflict);

  WriteFile(save_path, "SAVE_V1_PARTY:HACKED_BY_SOMETHING_ELSE");
  auto conflict_after = manager.CheckForExternalConflict();
  std::cout << "Conflict check (after external write): conflict=" << conflict_after.conflict
            << " detail=\"" << conflict_after.detail << "\"\n";
  assert(conflict_after.conflict);

  // --- Restore: roll back to the trade's post-completion backup ---------
  auto backups = manager.ListBackups();
  std::cout << "Total backups before restore: " << backups.size() << "\n";
  const save::BackupEntry* trade_backup = nullptr;
  for (const auto& entry : backups) {
    if (entry.tag == "post_trade_completed") {
      trade_backup = &entry;
      break;
    }
  }
  assert(trade_backup != nullptr);

  auto restore_result = manager.RestoreBackup(*trade_backup);
  std::cout << "Restore post_trade_completed: ok=" << restore_result.ok << "\n";
  assert(restore_result.ok);
  std::cout << "Save contents after restore: \"" << ReadFile(save_path) << "\"\n";
  assert(ReadFile(save_path) == "SAVE_V1_PARTY:RAICHU,CHARMANDER");

  // Rotation: max_backups_per_save was 5, so old backups get pruned as new
  // ones accumulate.
  auto final_backups = manager.ListBackups();
  std::cout << "Final backup count: " << final_backups.size() << " (max 5)\n";
  assert(static_cast<int>(final_backups.size()) <= 5);

  fs::remove_all(temp_dir, ec);
  std::cout << "All save_manager checks passed.\n";
  return 0;
}
