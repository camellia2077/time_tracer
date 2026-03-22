# 多端 Config 分发机制（Windows CLI / Android）

本文说明当前项目如何把 `assets/tracer_core/config` 作为单一事实来源，分发到各表现层运行时目录。

## 0. 目录职责边界

1. Canonical source：`assets/tracer_core/config`
2. Generated runtime copy：
   - `apps/cli/windows/rust/runtime/config`
   - `apps/android/runtime/src/main/assets/tracer_core/config`
3. 维护规则：
   - 修改共享 config 时，只改 `assets/tracer_core/config`
   - app 内 runtime config 目录只允许由同步流程刷新，不应手工作为源头维护
4. 非运行时设计参考文件（例如品牌 SVG、图标探索稿）不属于 `assets/`；
   其长期归属是 `design/branding/**`

## 1. 单一事实来源

1. 源配置目录（唯一 source of truth）：`assets/tracer_core/config`
2. 分发入口脚本：`tools/platform_config/run.py`
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

1. 读取 source bundle：`assets/tracer_core/config/meta/bundle.toml`
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
   - 校验 `config.toml` 关键键（如 `defaults.*`、`converter.interval_config`、`reports.markdown`）
4. 结构化观测日志
   - 每次同步输出：`sync_report=<json>`
   - 字段包含：`target/source/output/planned_files/added/changed/removed/cache_hit/applied/duration_ms`

## 6. 路径常量治理

为避免路径硬编码漂移，已统一收敛到：

1. `tools/platform_paths.py`
   - `tracer_core_config_root(repo_root)`
   - `windows_cli_config_root(repo_root)`
   - `android_config_root(repo_root)`

`platform_config/run.py` 与 `toolchain config_sync.py` 均使用该模块提供的路径函数。

## 7. 常用命令

1. Windows dry-run：
```bash
python tools/platform_config/run.py --target windows
```

2. Android apply：
```bash
python tools/platform_config/run.py --target android --apply
```

3. 构建时自动触发（示例）：
```bash
python tools/run.py configure --app tracer_windows_cli --build-dir build_fast
python tools/run.py build --app tracer_android --profile fast
```

## 8. 相关文档

1. Android 资产生命周期：`docs/time_tracer/clients/android_ui/specs/CONFIG_ASSET_LIFECYCLE.md`
2. Windows CLI 文档域：`docs/time_tracer/clients/windows_cli/README.md`

