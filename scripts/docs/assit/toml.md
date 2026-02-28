## TOML 配置生成与传递流程（当前）

### Windows CLI（当前推荐）
- Agent 只需要调用一条 verify 命令：
  - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`
- 这条命令会自动完成：
  1. 触发 build（不在 CMake 中写死 config 路径）。
  2. build 阶段自动调用 `scripts/platform_config/run.py`，生成并同步 Windows 侧配置。
  3. 自动向 CMake 注入 `TRACER_WINDOWS_CONFIG_SOURCE_DIR`。
  4. build 完成后自动运行测试（tracer_windows_cli 套件，core + windows cli 集成）。

也就是说，Windows 现在是 `verify` 单入口，不再要求 agent 手动分开执行 build 和 test。

### Android
- `scripts/run.py` 执行 Android build 时，会自动调用 `scripts/platform_config/run.py`。
- 自动传递 Android 配置根路径（Gradle 属性），使用生成后的 Android TOML 配置。
