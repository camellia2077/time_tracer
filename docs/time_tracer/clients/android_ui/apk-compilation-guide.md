# Android STL = c++_static (Python Entry)
# STL is configured in:
# apps/tracer_android/runtime/build.gradle.kts
# arguments += listOf("-DANDROID_STL=c++_static")

# Release (Signed, Native Optimization Enabled) - Recommended (repo root)
python .\scripts\run.py build --app tracer_android --profile android_release

# Release signing setup (local, untracked)
# 1. Copy:
#    apps/tracer_android/keystore.properties.example
#    -> apps/tracer_android/keystore.properties
# 2. Fill:
#    STORE_FILE
#    STORE_PASSWORD
#    KEY_ALIAS
#    KEY_PASSWORD
#
# CI can provide the same values through environment variables:
#    TT_ANDROID_STORE_FILE
#    TT_ANDROID_STORE_PASSWORD
#    TT_ANDROID_KEY_ALIAS
#    TT_ANDROID_KEY_PASSWORD

# Troubleshooting Only: Native No-Optimization
python .\scripts\run.py build --app tracer_android --profile android_release_no_opt

# Important:
# tracer_android uses Gradle backend, so `--cmake-args` is ignored in `tools/run.py`.
# If you need to change STL mode, modify runtime/build.gradle.kts first, then run python command above.

# Direct Gradle (apps/tracer_android)
.\gradlew.bat :app:assembleRelease -PtimeTracerDisableNativeOptimization=false

# Debug (Fast & Debuggable)
.\gradlew.bat :app:assembleDebug
