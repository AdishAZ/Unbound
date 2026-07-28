package com.unboundmp.client

/**
 * Thin Kotlin-side mirror of jni_bridge.cpp - one function per JNI export,
 * no logic of its own. Keeping this a dumb pass-through (rather than
 * adding Android-side business logic here) means the actual behavior
 * being tested is always in the C++ layer's own examples
 * (save_manager_example, touch_input_example, asset_path_example), not
 * duplicated or reinterpreted here.
 */
object NativeBridge {
    init {
        System.loadLibrary("unboundmp_jni")
    }

    // session_kind constants mirror unboundmp::save::LinkKind
    // (cpp/include/save/save_types.h) - kept in sync by hand since this
    // bridge doesn't share an enum definition across the JNI boundary.
    const val SESSION_KIND_UNSPECIFIED = 0
    const val SESSION_KIND_TRADE = 1
    const val SESSION_KIND_BATTLE = 2

    // phase constants mirror unboundmp::input::TouchPhase
    // (cpp/include/input/touch_input_mapper.h).
    const val TOUCH_PHASE_DOWN = 0
    const val TOUCH_PHASE_MOVE = 1
    const val TOUCH_PHASE_UP = 2
    const val TOUCH_PHASE_CANCEL = 3

    external fun nativeInit(appPrivateDir: String)

    external fun nativeOnTouchEvent(pointerId: Int, x: Float, y: Float, phase: Int): Int
    external fun nativeResetTouch()

    external fun nativeRomCachePathFor(romIdentifier: String): String
    external fun nativeInitSaveManager(savePath: String): Boolean
    external fun nativeLastError(): String
    external fun nativeBeginLinkSession(sessionId: Int, sessionKind: Int): Boolean
    external fun nativeEndLinkSession(sessionId: Int, sessionKind: Int, completed: Boolean): Boolean
}
