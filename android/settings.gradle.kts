// Android build scaffolding for the Milestone 13 Android-support
// foundation. This is a project skeleton, not a finished app: it wires the
// existing cpp/ libraries into an NDK build and stubs the JNI bridge, but
// does not yet render a game screen. See android/README.md for what is and
// isn't implemented, and why.
pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "unboundmp-android"
include(":app")
