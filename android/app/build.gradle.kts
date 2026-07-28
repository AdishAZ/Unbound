plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.unboundmp.client"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.unboundmp.client"
        // GBA games run comfortably on modest hardware; minSdk 24 (Android
        // 7.0) covers effectively all devices capable of running mGBA at
        // full speed while keeping the NDK ABI/API surface manageable.
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "0.1.0-foundation"

        externalNativeBuild {
            cmake {
                // C++20 to match cpp/CMakeLists.txt; arguments here mirror
                // (not duplicate) the desktop build's options.
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DUNBOUNDMP_BUILD_EXAMPLES=OFF",
                    // Real libmgba-for-Android integration is a follow-up
                    // milestone (see android/README.md) - off by default so
                    // this module configures/builds without it present.
                    "-DUNBOUNDMP_WITH_MGBA=OFF"
                )
            }
        }

        ndk {
            // arm64-v8a covers effectively all Android devices sold in the
            // last several years; armeabi-v7a is included for older/low-end
            // hardware where GBA emulation is still very feasible.
            abiFilters += listOf("arm64-v8a", "armeabi-v7a")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
}
