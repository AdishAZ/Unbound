// JNI bridge between MainActivity.kt (and, in future milestones, a
// surface-rendering layer) and the shared cpp/ libraries.
//
// This file intentionally holds no game logic - every function below does
// argument marshalling and then calls straight into a class that already
// has its own desktop smoke-test example (save_manager_example.cpp,
// touch_input_example.cpp, asset_path_example.cpp). If behavior looks
// wrong on-device, the bug is almost certainly reproducible in one of
// those examples first, which is the point of keeping this thin.
//
// Not yet implemented here (later milestones - see android/README.md):
//   - Emulator core lifecycle (needs UNBOUNDMP_WITH_MGBA's Android build)
//   - NetworkManager wiring (gated behind UNBOUNDMP_ANDROID_WITH_NETWORK -
//     see app/src/main/cpp/CMakeLists.txt for why)
//   - Any drawing/surface code - there is no rendering milestone yet on
//     desktop either (see /README.md "Not yet implemented")
#include <jni.h>

#include <memory>
#include <mutex>
#include <string>

#include "input/touch_input_mapper.h"
#include "platform/asset_path_resolver.h"
#include "save/save_manager.h"

namespace {

// One mapper/resolver/manager per process, matching how a single native
// client would own exactly one of each. Guarded by a mutex because JNI
// calls can arrive from more than one Java thread (touch events typically
// arrive on the UI thread; save operations might be triggered from a
// background thread watching for LinkSessionEnd).
std::mutex g_state_mutex;
std::unique_ptr<unboundmp::input::TouchInputMapper> g_touch_mapper;
std::unique_ptr<unboundmp::platform::AssetPathResolver> g_asset_resolver;
std::unique_ptr<unboundmp::save::SaveManager> g_save_manager;

std::string JStringToStd(JNIEnv* env, jstring value) {
  if (value == nullptr) return {};
  const char* chars = env->GetStringUTFChars(value, nullptr);
  std::string result(chars);
  env->ReleaseStringUTFChars(value, chars);
  return result;
}

unboundmp::input::TouchPhase PhaseFromInt(jint phase) {
  switch (phase) {
    case 0: return unboundmp::input::TouchPhase::kDown;
    case 1: return unboundmp::input::TouchPhase::kMove;
    case 2: return unboundmp::input::TouchPhase::kUp;
    default: return unboundmp::input::TouchPhase::kCancel;
  }
}

}  // namespace

extern "C" {

// --- Lifecycle --------------------------------------------------------

JNIEXPORT void JNICALL
Java_com_unboundmp_client_NativeBridge_nativeInit(JNIEnv* env, jobject /*thiz*/,
                                                    jstring app_private_dir) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  g_touch_mapper = std::make_unique<unboundmp::input::TouchInputMapper>();
  g_asset_resolver = std::make_unique<unboundmp::platform::AssetPathResolver>(
      JStringToStd(env, app_private_dir));
  g_save_manager = std::make_unique<unboundmp::save::SaveManager>();
}

// --- Touch input --------------------------------------------------------
// x/y are normalized [0,1] against the current surface size - MainActivity
// converts raw MotionEvent pixel coordinates before calling this, keeping
// TouchInputMapper itself resolution-independent (see touch_input_mapper.h).
// Returns the resulting GBA held-button bitmask (emulator::InputState::held_mask).

JNIEXPORT jint JNICALL
Java_com_unboundmp_client_NativeBridge_nativeOnTouchEvent(JNIEnv* /*env*/, jobject /*thiz*/,
                                                            jint pointer_id, jfloat x, jfloat y,
                                                            jint phase) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (!g_touch_mapper) return 0;
  unboundmp::input::TouchEvent event{{pointer_id, x, y}, PhaseFromInt(phase)};
  return static_cast<jint>(g_touch_mapper->OnTouchEvent(event).held_mask);
}

JNIEXPORT void JNICALL
Java_com_unboundmp_client_NativeBridge_nativeResetTouch(JNIEnv* /*env*/, jobject /*thiz*/) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (g_touch_mapper) g_touch_mapper->Reset();
}

// --- Asset paths + save sync --------------------------------------------
// `rom_identifier` is whatever stable name/URI-derived string MainActivity
// has for the ROM the player picked via the system file picker (Storage
// Access Framework) - see platform/asset_path_resolver.h for why this
// can't just be a filesystem path on Android the way it is on desktop.

JNIEXPORT jstring JNICALL
Java_com_unboundmp_client_NativeBridge_nativeRomCachePathFor(JNIEnv* env, jobject /*thiz*/,
                                                               jstring rom_identifier) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (!g_asset_resolver) return env->NewStringUTF("");
  const std::string path =
      g_asset_resolver->RomCachePathFor(JStringToStd(env, rom_identifier));
  return env->NewStringUTF(path.c_str());
}

// Initializes SaveManager for the given (already-resolved-to-a-real-path)
// save file. Returns true on success; on failure the reason is logged via
// Log.e from Kotlin using nativeLastError().
std::string g_last_error;

JNIEXPORT jboolean JNICALL
Java_com_unboundmp_client_NativeBridge_nativeInitSaveManager(JNIEnv* env, jobject /*thiz*/,
                                                               jstring save_path) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (!g_save_manager) return JNI_FALSE;
  auto result = g_save_manager->InitializeForSavePath(JStringToStd(env, save_path));
  if (!result) g_last_error = result.message;
  return result.ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL
Java_com_unboundmp_client_NativeBridge_nativeLastError(JNIEnv* env, jobject /*thiz*/) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  return env->NewStringUTF(g_last_error.c_str());
}

// session_kind: 0 = unspecified, 1 = trade, 2 = battle (mirrors
// save::LinkKind - see save/save_types.h).
JNIEXPORT jboolean JNICALL
Java_com_unboundmp_client_NativeBridge_nativeBeginLinkSession(JNIEnv* /*env*/, jobject /*thiz*/,
                                                                jint session_id,
                                                                jint session_kind) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (!g_save_manager) return JNI_FALSE;
  auto result = g_save_manager->BeginLinkSession(
      static_cast<uint32_t>(session_id), static_cast<unboundmp::save::LinkKind>(session_kind));
  if (!result) g_last_error = result.message;
  return result.ok ? JNI_TRUE : JNI_FALSE;
}

// Returns true if the session's save changes were confirmed persisted to
// disk (SaveSyncResult::persisted) - see save/save_manager.h for what that
// guarantees and doesn't.
JNIEXPORT jboolean JNICALL
Java_com_unboundmp_client_NativeBridge_nativeEndLinkSession(JNIEnv* /*env*/, jobject /*thiz*/,
                                                              jint session_id, jint session_kind,
                                                              jboolean completed) {
  std::lock_guard<std::mutex> lock(g_state_mutex);
  if (!g_save_manager) return JNI_FALSE;
  auto result = g_save_manager->EndLinkSession(static_cast<uint32_t>(session_id),
                                                static_cast<unboundmp::save::LinkKind>(session_kind),
                                                completed == JNI_TRUE);
  if (!result.ok) g_last_error = result.message;
  return result.persisted ? JNI_TRUE : JNI_FALSE;
}

}  // extern "C"
