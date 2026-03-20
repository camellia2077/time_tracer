# Native Modules Guide

本文档约束 `time_tracer` 在 C++20 named modules 路径下的结构、命名与准入规则。

## 1. 默认开关策略

1. `apps/tracer_core_shell` 现已进入 modules-only 默认路径；
   受支持工具链下直接按 modules-only 配置。
2. `TT_ENABLE_CPP20_MODULES`
   已从受支持入口中移除；
   shell configure
   直接要求
   modules-only toolchain。
3. Android Gradle
   也不再承认
   `-PtimeTracerEnableCpp20Modules`
   作为受支持入口；
   native build
   不再透传该兼容属性。
4. 当前支持基线是：
   - CMake `>= 3.28`
   - `Clang` / `GNU` / `MSVC` C++ 编译器
   - 不满足基线时直接在配置阶段失败
5. `libs/tracer_core`
   已完成
   `pure internal module-only`
   closeout：
   core-internal
   主路径
   不再承认
   header fallback
   或 dual-track
   declaration ownership。

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
   - `tracer.adapters.io.runtime`
   - `tracer.adapters.io.utils.file_utils`
3. `libs/tracer_core/src/shared/modules/`
   - `tracer.core.shared`
   - `tracer.core.shared.string_utils`
   - `tracer.core.shared.period_utils`
   - `tracer.core.shared.exceptions`
   - `tracer.core.shared.exit_codes`
4. `libs/tracer_core/src/domain/modules/`
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
5. `libs/tracer_core/src/application/modules/`
   - `tracer.core.application`（聚合）
   - `tracer.core.application.use_cases.*`
   - `tracer.core.application.service.converter`
   - `tracer.core.application.importer.service`
   - `tracer.core.application.pipeline.*`
   - `tracer.core.application.workflow_handler`
6. `libs/tracer_core/src/infrastructure/modules/`
   - `tracer.core.infrastructure`（聚合）
   - 按 family 子目录组织：
     - `logging/**`
     - `platform/**`
     - `config/**`
     - `persistence/**`
     - `query/**`
     - `reports/**`

### 2.1 迁移后构建归属（2026-03-05）

1. `tracer.core.shared.*` 的构建归属在 `libs/tracer_core/CMakeLists.txt`（`tracer_core_shared_lib`）。
2. `tracer.core.domain.*` 的构建归属在 `libs/tracer_core/CMakeLists.txt`（`tracer_core_domain_lib`）。
3. `tracer.core.application.*` 的构建归属在 `libs/tracer_core/CMakeLists.txt`（`tracer_core_application_lib`）。
4. `tracer.core.infrastructure.*` 低耦合模块构建归属在 `libs/tracer_core/CMakeLists.txt`（`tracer_core_infrastructure_lite_lib`）。
5. `infrastructure` 剩余业务实现（含 reports/query/persistence/config/crypto）的构建归属在
   `libs/tracer_core/infrastructure/CMakeLists.txt`（`tti`/`ttri`/`reports_*`）。
6. `libs/tracer_core/src/infrastructure/CMakeLists.txt` 仅保留壳层接线，不再维护业务源码清单。

## 3. 命名规则

1. 模块接口文件统一放在 `src/modules/**/*.cppm`。
2. 文件名与 module 名保持一一映射：
   - `tracer.adapters.io.core.reader.cppm` <-> `export module tracer.adapters.io.core.reader;`
3. 根聚合模块负责 `export import` 子模块：
   - 例如 `tracer.adapters.io.cppm` 只做聚合导出，不承载重逻辑。
4. 兼容桥接命名统一使用 `mod*` 命名空间（例如 `modcore` / `modutils` / `moderrors`）。
5. 现阶段不在接口单元导出高耦合 JSON 细节类型；高耦合细节保留在实现单元。

## 4. 新代码准入规则

1. 已模块化子域新增公共能力默认走 module-first：
   - 优先新增/扩展 `src/modules/**/*.cppm`；
   - 仅在 C ABI 边界、第三方受限头或明确增量迁移阶段保留 header 桥接。
2. 禁止新增“纯转发头”（仅 `#include` 其它头、无声明/逻辑），
   除非有跨仓外部 contract 要求且在变更说明中注明保留原因。
3. 每个新增模块面必须有测试：
   - 至少一个 modules smoke test；
   - 若影响稳定 ABI / JNI / runtime boundary，
     必须新增或更新对应 contract regression。
4. 改动合入前必须跑：
   - modules-only 主路径
   - 对稳定 contract surface
     的 ABI / JNI / runtime regression
5. 不再新增 `modules_off` validate 计划、脚本开关或回退说明。
6. Android 主路径默认按 modules-only 验证；
   不保留模块总开关回归入口。
7. 构建与验证统一通过 `python tools/run.py ...`，禁止手工 cmake/ninja 流程作为主路径。
8. 不得为
   `libs/tracer_core`
   新增新的 internal declaration owner
   header fallback
   或重新引入
   `TT_FORCE_LEGACY_HEADER_DECLS`
   主路径；
   若 ABI / host / toolchain
   需要声明面，
   必须将其记录为显式 boundary surface。

## 5. 推荐验证命令

1. tracer_core_shell focused validate：
   - `python tools/run.py validate --plan <plan.toml> --paths <touched paths>`
2. Shell quick batch verify：
   - `python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise`
3. Android edit/build verify：
   - `python tools/run.py build --app tracer_android --profile android_edit`
4. Android CI 样式单元回归：
   - `python tools/run.py verify --app tracer_android --profile android_ci --scope unit --concise`
