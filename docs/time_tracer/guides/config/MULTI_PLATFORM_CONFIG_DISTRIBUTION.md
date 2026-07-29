# 多端 Config 分发机制（Windows CLI / Android）

本文说明当前项目如何从受 profile 管理的 Core 配置资产分发到各表现层运行时目录。alias TOML 只使用 canonical-key-first 格式，canonical key 位于左侧，alias 位于右侧字符串数组中；旧的 `alias = "canonical"` 格式不再使用。

## 0. 目录职责边界

1. Test/package source：`assets/tracer_core/config_test`
2. Distribution source：`assets/tracer_core/config_distribution`
3. Generated runtime copy：
   - `apps/cli/windows/rust/runtime/config`
   - `apps/android/runtime/src/main/assets/tracer_core/config`
4. 维护规则：
   - 按用途修改 `config_test` 或 `config_distribution`，不修改 generated runtime copy
   - app 内 runtime config 目录只允许由同步流程刷新，不应手工作为源头维护
5. 非运行时设计参考文件（例如品牌 SVG、图标探索稿）不属于 `assets/`；
   其长期归属是 `design/branding/**`

## 1. 单一事实来源

1. 源配置目录按 profile 选择：测试/打包使用 `assets/tracer_core/config_test`，
   分发使用 `assets/tracer_core/config_distribution`
2. 分发入口模块：`tools.platform_config.run`
3. 构建链路触发点：`tools/run.py build|configure` -> `tools/toolchain/commands/cmd_build/common/config_sync.py`

## 2. 目标目录（当前）

1. Windows CLI generated copy：`apps/cli/windows/rust/runtime/config`
2. Android Runtime generated copy：`apps/android/runtime/src/main/assets/tracer_core/config`

## 3. 触发策略

1. 仅当 app 在 `tools/toolchain/config/apps.toml` 配置了 `config_sync_target` 时，构建前自动分发。
2. 当前启用项：
   - `[apps.tracer_windows_cli] config_sync_target = "windows"`
   - `[apps.tracer_android] config_sync_target = "android"`
3. `tracer_core` 本身未配置 `config_sync_target`，单独构建 core 不触发分发。

## 4. 分发流程（构建时）

1. 读取所选 source bundle：`<source_root>/meta/bundle.toml`
2. 根据 target 生成计划文件集（windows/android）
3. 执行增量判断（输入哈希 + 状态文件）
4. 命中缓存则跳过写入；未命中则执行原子写入
5. 写入后执行强校验，失败即终止构建

## 5. 已启用的可靠性机制

1. 增量同步（缓存）
   - 状态文件：`<output>/meta/sync_state.json`
   - 哈希维度：target + source_root + planned file hashes
   - 缓存命中时不写盘，直接返回
2. 原子写入与回滚
   - 先写 staging 目录
   - 成功后原子切换到输出目录
   - 失败时回滚 backup，避免半成品目录
3. 强校验门禁
   - 校验所有计划文件存在且字节一致
   - 校验 `meta/bundle.toml` 的 `schema_version/profile/bundle_name`
   - 校验 alias TOML 全部为 canonical key + alias 数组格式；分发 alias
     目录不包含用户业务 alias，只保留空白 `default.toml` 和必须保留的
     `_system.toml`。`_system.toml` 是系统业务配置，包含
     `wake_keywords`、`sleep_project_path` 等睡眠时间推导规则，不能删除或
     视为空白 alias 文件
   - 校验 `config.toml` 关键键（如 `defaults.*`、`converter.main_config`、
     `reports.markdown.root/default_locale/supported_locales` 以及 Typst/LaTeX
     的报告根目录）
   - `meta/bundle.toml` 只声明包元数据和 `file_list`；路径配置统一来自
     `config.toml`
4. 结构化观测日志
   - 每次同步输出：`sync_report=<json>`
   - 字段包含：`target/source/output/planned_files/added/changed/removed/cache_hit/applied/duration_ms`

## 6. 路径常量治理

为避免路径硬编码漂移，已统一收敛到：

1. `tools/platform_paths.py`
   - `tracer_core_config_root(repo_root)`
   - `windows_cli_config_root(repo_root)`
   - `android_config_root(repo_root)`

`python -m tools.platform_config.run` 与 `toolchain config_sync.py` 均使用该模块提供的路径函数。

## 7. 常用命令

1. Windows dry-run：
```bash
python -m tools.platform_config.run --target windows
```

2. Android apply：
```bash
python -m tools.platform_config.run --target android --apply
```

3. 构建时自动触发（示例）：
```bash
python tools/run.py configure --app tracer_windows_cli --build-dir build_fast
python tools/run.py build --app tracer_android --profile fast --config-profile test
```

## 8. 相关文档

1. Android 资产生命周期：`docs/time_tracer/clients/android_ui/specs/CONFIG_ASSET_LIFECYCLE.md`
2. Windows CLI 文档域：`docs/time_tracer/clients/windows_cli/README.md`
