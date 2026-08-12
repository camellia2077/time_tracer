## formatter task
cd apps/android
.\gradlew.bat ktlintFormat

## ktlint、lint 和 detekt

python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise
