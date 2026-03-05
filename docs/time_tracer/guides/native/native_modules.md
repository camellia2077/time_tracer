# Native Modules Guide

本文档约束 `time_tracer` 在 C++20 named modules 路径下的结构、命名与准入规则。

## 1. 默认开关策略

1. `apps/tracer_core` 在 Windows Host 与 Android 默认 `TT_ENABLE_CPP20_MODULES=ON`。
2. 任意平台都可通过 CMake 参数显式覆盖：
   - 开启：`-DTT_ENABLE_CPP20_MODULES=ON`
   - 关闭：`-DTT_ENABLE_CPP20_MODULES=OFF`
3. Android Gradle 路径可通过属性显式覆盖：
   - 开启：`-PtimeTracerEnableCpp20Modules=ON`
   - 关闭：`-PtimeTracerEnableCpp20Modules=OFF`
4. 任何问题可回退到 OFF 路径，保持传统 `.hpp + .cpp` 持续可编译。

## 2. 当前模块结构

1. `libs/tracer_transport/src/modules/`
   - `tracer.transport`
   - `tracer.transport.errors`
   - `tracer.transport.envelope`
2. `libs/tracer_adapters_io/src/modules/`
   - `tracer.adapters.io`
   - `tracer.adapters.io.core.reader`
   - `tracer.adapters.io.core.writer`
   - `tracer.adapters.io.core.fs`
   - `tracer.adapters.io.utils.file_utils`
3. `apps/tracer_core/src/shared/modules/`
   - `tracer.core.shared`
   - `tracer.core.shared.string_utils`
   - `tracer.core.shared.period_utils`
   - `tracer.core.shared.exceptions`
   - `tracer.core.shared.exit_codes`
   - `tracer.core.shared.ansi_colors`
4. `apps/tracer_core/src/domain/modules/`
   - `tracer.core.domain`（聚合）
   - `tracer.core.domain.ports.diagnostics`
   - `tracer.core.domain.repositories.project_repository`
   - `tracer.core.domain.model.*`
   - `tracer.core.domain.types.*`
   - `tracer.core.domain.errors.error_record`
   - `tracer.core.domain.reports.types.report_types`
   - `tracer.core.domain.reports.models.*`
   - `tracer.core.domain.logic`（聚合）
   - `tracer.core.domain.logic.converter.core`
   - `tracer.core.domain.logic.converter.log_processor`
   - `tracer.core.domain.logic.validator.common.*`
   - `tracer.core.domain.logic.validator.txt.*`
   - `tracer.core.domain.logic.validator.structure`
5. `apps/tracer_core/src/application/modules/`
   - `tracer.core.application`（聚合）
   - `tracer.core.application.use_cases.*`
   - `tracer.core.application.service.converter`
   - `tracer.core.application.importer.service`
   - `tracer.core.application.pipeline.*`
   - `tracer.core.application.workflow_handler`
6. `apps/tracer_core/src/infrastructure/modules/`
   - `tracer.core.infrastructure`（聚合）
   - `tracer.core.infrastructure.logging.*`
   - `tracer.core.infrastructure.platform.*.clock`
   - `tracer.core.infrastructure.config.static_converter_config_provider`

## 3. 命名规则

1. 模块接口文件统一放在 `src/modules/*.cppm`。
2. 文件名与 module 名保持一一映射：
   - `tracer.adapters.io.core.reader.cppm` <-> `export module tracer.adapters.io.core.reader;`
3. 根聚合模块负责 `export import` 子模块：
   - 例如 `tracer.adapters.io.cppm` 只做聚合导出，不承载重逻辑。
4. 兼容桥接命名统一使用 `mod*` 命名空间（例如 `modcore` / `modutils` / `moderrors`）。
5. 现阶段不在接口单元导出高耦合 JSON 细节类型；高耦合细节保留在实现单元。

## 4. 新代码准入规则

1. 已模块化子域新增公共能力默认走 module-first：
   - 优先新增/扩展 `src/modules/*.cppm`；
   - 仅在 C ABI 边界、第三方受限头或明确增量迁移阶段保留 header 桥接。
2. 禁止新增“纯转发兼容头”（仅 `#include` 其它头、无声明/逻辑），
   除非有跨仓外部兼容要求且在变更说明中注明保留原因。
3. 每个新增模块面必须有测试：
   - 至少一个 modules smoke test；
   - 若影响旧头调用，必须新增/更新 legacy 兼容测试。
4. 改动合入前必须跑双轨：
   - OFF 路径（默认/回退路径）
   - ON 路径（modules 路径）
5. Android 主路径默认按 ON 验证，同时保留 `OFF` 回退能力：
   - `-PtimeTracerEnableCpp20Modules=OFF`
6. 构建与验证统一通过 `python scripts/run.py ...`，禁止手工 cmake/ninja 流程作为主路径。

## 5. 推荐验证命令

1. Core ON（默认）回归：
   - `python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_core_phase6_on --concise`
2. Core OFF 回退回归：
   - `python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_core_phase6_off --cmake-args=-DTT_ENABLE_CPP20_MODULES=OFF --concise`
3. Android ON（默认）回归：
   - `python scripts/run.py post-change --app tracer_android --profile fast --run-tests always --concise`
4. Android CI 样式单元回归：
   - `python scripts/run.py verify --app tracer_android --profile android_ci --scope unit --concise`
