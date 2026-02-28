# Android STL = c++_static (Python Entry)
# STL is configured in:
# apps/tracer_android/runtime/build.gradle.kts
# arguments += listOf("-DANDROID_STL=c++_static")

# Release (Signed, Native Optimization Enabled) - Recommended (repo root)
python .\scripts\run.py build --app tracer_android --profile android_release

# Troubleshooting Only: Native No-Optimization
python .\scripts\run.py build --app tracer_android --profile android_release_no_opt

# Important:
# tracer_android uses Gradle backend, so `--cmake-args` is ignored in `scripts/run.py`.
# If you need to change STL mode, modify runtime/build.gradle.kts first, then run python command above.

# Direct Gradle (apps/tracer_android)
.\gradlew.bat :app:assembleRelease -PtimeTracerDisableNativeOptimization=false

# Debug (Fast & Debuggable)
.\gradlew.bat :app:assembleDebug
