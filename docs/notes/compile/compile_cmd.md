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
