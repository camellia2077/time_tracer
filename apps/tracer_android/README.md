# tracer_android

Android host app for `apps/time_tracer` (`Jetpack Compose + JNI`).

## Architecture

Current layering:

- `app`: composition root + main UI host
- `contract`: stable `RuntimeGateway` + DTO contracts
- `feature-data`: data screen
- `feature-report`: report screen
- `feature-record`: record/txt screens
- `runtime`: JNI + runtime implementation

Dependency direction:

- `app -> contract + runtime + feature-*`
- `feature-* -> contract`
- `runtime -> contract`

## Runtime Data Layout

Bundled assets location:

- `runtime/src/main/assets/time_tracer/**`

On app startup, assets are copied to:

- `${filesDir}/time_tracer/**`

Then runtime initializes via:

- `nativeInit(dbPath, outputRoot, converterConfigTomlPath)`

`converterConfigTomlPath`:

- `${filesDir}/time_tracer/config/converter/interval_processor_config.toml`

## Setup

1. Create `apps/tracer_android/local.properties`:
   ```properties
   sdk.dir=C\:\\Application\\Android\\SDK
   # cmake.dir=... (optional, if not in PATH or SDK)
   ```

## Build

Powershell:
```powershell
cd apps/tracer_android
# Ensure JAVA_HOME points to JDK 17+ (e.g. Android Studio JBR)
$env:JAVA_HOME='C:\Application\Android\as\jbr'
.\gradlew :app:assembleDebug
```

NDK requirement:

- `29.0.14206865`

Native CMake entry:

- `runtime/src/main/cpp/CMakeLists.txt`

## Regression

Use:

- `apps/tracer_android/REGRESSION_CHECKLIST.md`

