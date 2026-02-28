# scripts 重构基线（Step 0）

基线时间：`2026-02-18 01:26:49 +08:00`

## 回归命令与结果

1. `python scripts/run.py self-test`
   - 结果：`PASS`（17/17）
2. `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`
   - 结果：`PASS`（tracer_windows_cli suite: 124/124）
3. `python scripts/run.py verify --app log_generator --build-dir build_fast --concise`
   - 结果：`PASS`（log_generator suite: 2/2）
4. `python scripts/run.py build --app tracer_android --profile fast`
   - 结果：`PASS`（Gradle assembleDebug 成功）

## 备注

- 为避免 `ninja: no work to do` 时 `build/bin/config` 保留旧配置，
  已在 `scripts/toolchain/commands/build.py` 增加 Windows 运行时配置目录的构建后强制同步。
