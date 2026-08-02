## release (default, LTO off)

### core
```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
```

### rust-cli
```powershell
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

### core+rust-cli
```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

### user activity hierarchy

`config/user/activity_hierarchy/**` is a repository-side user-configuration
entry and is excluded from Android and Windows CLI compilation outputs. It is
kept to document the runtime-owned activity hierarchy business. User hierarchy
TOML files are imported or created in the application's private user-data
directory at runtime.

The canonical examples used by tests are kept separately under
`test/data/activity_hierarchy` and are not copied into release artifacts.

## release + lto (explicit opt-in)

### core
```powershell
python tools/run.py build --app tracer_core --profile release_bundle_ci_no_pch --build-dir build_lto --runtime-platform windows --cmake-args=-DTT_ENABLE_LTO=ON
```

### rust-cli
```powershell
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle_ci_no_pch --build-dir build_lto --runtime-platform windows
```

### core+rust-cli
```powershell
python tools/run.py build --app tracer_core --profile release_bundle_ci_no_pch --build-dir build_lto --runtime-platform windows --cmake-args=-DTT_ENABLE_LTO=ON
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle_ci_no_pch --build-dir build_lto --runtime-platform windows
```

## note

`TT_ENABLE_LTO` is now the only supported opt-in switch.
Default release profiles still keep `ENABLE_LTO=OFF`.
