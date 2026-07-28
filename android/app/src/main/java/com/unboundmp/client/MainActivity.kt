package com.unboundmp.client

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import java.io.File
import java.io.FileOutputStream

/**
 * Foundation-milestone activity. What this does today:
 *   - lets the player pick their own ROM file via the system file picker
 *     (Storage Access Framework) - never bundles, downloads, or assumes a
 *     ROM is present
 *   - copies that ROM's bytes, unmodified, into the app-private cache path
 *     AssetPathResolver defines (RomCachePathFor) - a real filesystem path
 *     the emulator core will need once it's built for Android, since a
 *     content:// URI generally isn't one
 *   - initializes SaveManager against the matching save path and exercises
 *     a scripted trade checkpoint so the whole save-sync path (see
 *     save/save_manager.h) is provably wired end-to-end on-device, not
 *     just in the desktop example
 *   - turns raw touch events into normalized coordinates and feeds them to
 *     TouchInputMapper via the JNI bridge, displaying the resulting held-
 *     button mask as text
 *
 * What this deliberately does NOT do yet (see android/README.md):
 *   - draw a game screen (no emulator core, no rendering milestone on
 *     desktop yet either)
 *   - connect to the multiplayer server (NetworkManager is gated behind
 *     UNBOUNDMP_ANDROID_WITH_NETWORK - see app/src/main/cpp/CMakeLists.txt)
 *   - render the on-screen touch control layout graphically - it reacts to
 *     touches in DefaultLayout()'s regions but doesn't paint them
 */
class MainActivity : AppCompatActivity() {

    private lateinit var statusText: TextView

    private val pickRom =
        registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
            if (uri != null) onRomPicked(uri)
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        NativeBridge.nativeInit(filesDir.absolutePath)

        statusText = TextView(this).apply {
            text = "No ROM loaded. Tap to pick a ROM file."
            textSize = 16f
            setPadding(32, 32, 32, 32)
        }
        setContentView(statusText)

        statusText.setOnClickListener {
            // image/rom and */* both show up oddly across OEM file pickers
            // for .gba files with no registered MIME type, so accept
            // anything and let the user navigate to their ROM.
            pickRom.launch(arrayOf("*/*"))
        }

        // Touch harness: while no ROM is loaded, still exercise the touch
        // input pipeline end-to-end so this is testable on-device even
        // before ROM/save wiring completes.
        statusText.setOnTouchListener { view: View, event: MotionEvent ->
            handleTouchEvent(view, event)
            true
        }
    }

    private fun handleTouchEvent(view: View, event: MotionEvent) {
        val phase = when (event.actionMasked) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> NativeBridge.TOUCH_PHASE_DOWN
            MotionEvent.ACTION_MOVE -> NativeBridge.TOUCH_PHASE_MOVE
            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> NativeBridge.TOUCH_PHASE_UP
            else -> NativeBridge.TOUCH_PHASE_CANCEL
        }

        val actionIndex = event.actionIndex
        val pointerId = event.getPointerId(actionIndex)
        val normalizedX = event.getX(actionIndex) / view.width.toFloat()
        val normalizedY = event.getY(actionIndex) / view.height.toFloat()

        val heldMask = NativeBridge.nativeOnTouchEvent(pointerId, normalizedX, normalizedY, phase)
        Log.d(TAG, "touch phase=$phase held_mask=$heldMask")
    }

    private fun onRomPicked(uri: Uri) {
        // Stable-enough identifier for AssetPathResolver's namespacing
        // (see platform/asset_path_resolver.h) - the display name, not the
        // opaque content:// URI itself, which can change across launches.
        val displayName = uri.lastPathSegment ?: "rom"
        val romCachePath = NativeBridge.nativeRomCachePathFor(displayName)

        val cacheFile = File(romCachePath)
        cacheFile.parentFile?.mkdirs()

        // Copy the SAF-provided ROM's bytes into the cache path unchanged -
        // read-only, never written back to the URI, matching RomLoader's
        // "never modify the ROM" contract on desktop.
        contentResolver.openInputStream(uri)?.use { input ->
            FileOutputStream(cacheFile).use { output -> input.copyTo(output) }
        }

        val savePath = cacheFile.absolutePath.removeSuffix(".gba") + ".sav"
        val saveOk = NativeBridge.nativeInitSaveManager(savePath)
        if (!saveOk) {
            statusText.text = "Save manager init failed: ${NativeBridge.nativeLastError()}"
            return
        }

        statusText.text = "ROM cached at $romCachePath\nSave manager ready at $savePath\n" +
            "Touch anywhere to exercise the touch-control pipeline."
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
