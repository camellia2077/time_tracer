# Release (Signed, Native Optimization Enabled)
# Recommended (repo root)
python .\scripts\run.py build --app tracer_android --profile android_release

# Troubleshooting Only: Native No-Optimization
python .\scripts\run.py build --app tracer_android --profile android_release_no_opt

# Direct Gradle (apps/tracer_android)
.\gradlew.bat :app:assembleRelease -PtimeTracerDisableNativeOptimization=false

# Debug (Fast & Debuggable)
.\gradlew.bat :app:assembleDebug
