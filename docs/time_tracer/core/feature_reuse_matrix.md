# Core Feature Reuse Matrix

## 元信息
1. 版本：`v1`
2. 更新日期：`2026-02-25`
3. 目标：记录“`tracer_core` 已实现，但当前仅 Android 复用、`windows_cli` 尚未复用”的能力状态。

## 状态枚举
1. `not_started`：未开始接入
2. `in_progress`：接入中
3. `done`：已完成并可用
4. `blocked`：受架构/产品决策阻塞

## 复用矩阵

| Capability | Core Status | Android Status | Windows CLI Status | Reason (why not adopted) | Contract/Test Coverage | Next Action |
| --- | --- | --- | --- | --- | --- | --- |
| `ingest_mode=single_txt_replace_month`（同年月替换） | `done` | `done` | `not_started` | `ingest` 命令当前未暴露 ingest mode 参数，默认标准模式。 | Core: `apps/tracer_core/src/api/core_c/tracer_core_c_api_internal.cpp`; Android bridge: `apps/tracer_core/src/api/android/native_bridge_calls.cpp`; Android runtime test: `apps/tracer_android/runtime/src/test/java/com/example/tracer/RuntimeIngestServiceTest.kt` | Windows CLI 评估是否增加 `--ingest-mode single_txt_replace_month`。 |
| 单次调用导入 `.tracer`（解密后按月替换入库）能力基元（`decrypt + single_txt_replace_month ingest`） | `done` | `done` | `not_started` | Windows 目前是两步命令（`crypto decrypt` + `ingest`），没有一体化导入命令。 | CLI crypto tests: `test/suites/tracer_windows_cli/tests/commands_crypto.toml`; Android 导入说明：`docs/time_tracer/clients/android_ui/features.md` | 后续若需要 Windows 一键导入，可新增 `crypto import` 或 `ingest --from-tracer`。 |
| Android JNI 专用入口 `nativeIngestSingleTxtReplaceMonth` | `done` | `done` | `blocked` | 该入口是 Android Native Bridge 专用，Windows CLI 不走 JNI。 | Registration: `apps/tracer_core/src/api/android/native_bridge_registration.cpp`; Bridge decl: `apps/tracer_android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt` | 保持 Android-only；Windows 若要复用，走 CLI 参数层接入 core runtime API。 |

## 维护规则
1. 每次新增 core 能力时，必须评估 Android/Windows CLI 复用状态，并同步本表。
2. 每次客户端接入完成时，必须更新对应 `Status` 与 `Contract/Test Coverage`。
3. 对 `blocked` 条目，必须写明阻塞原因和解除条件，避免长期不透明。
