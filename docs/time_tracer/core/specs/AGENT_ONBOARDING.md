# tracer_core Agent Onboarding

本文档用于帮助 Agent 或新同学在 5 分钟内定位 `tracer_core` 的修改入口，减少全局搜索。

## 1. 建议阅读顺序（5 分钟路径）
1. `docs/time_tracer/core/README.md`
2. `docs/time_tracer/core/architecture/README.md`
3. 本文档：`docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
4. 按需求阅读契约：
   - C ABI：`docs/time_tracer/core/contracts/c_abi.md`
   - 错误模型：`docs/time_tracer/core/contracts/error-model.md`
   - 加密进度：`docs/time_tracer/core/contracts/crypto/progress_callback_contract_v1.md`
   - 统计契约：`docs/time_tracer/core/contracts/stats/README.md`

## 2. 模块职责地图（src）
1. `apps/tracer_core/src/domain`
   - 领域模型、业务规则、领域端口，不依赖平台细节。
2. `apps/tracer_core/src/application`
   - 用例编排、pipeline、DTO、ports。
3. `apps/tracer_core/src/infrastructure`
   - IO、数据库、查询、报表、加密、配置加载等具体实现。
4. `apps/tracer_core/src/api/core_c`
   - 对外 C ABI 入口（Windows CLI 等 consumer 主要经由此层调用）。
5. `apps/tracer_core/src/api/android`
   - Android JNI 入口与桥接逻辑。
6. `apps/tracer_core/src/shared`
   - 跨层共用类型/工具（避免承载业务流程）。

## 3. 常见需求到文件定位（高频）
1. 改 C ABI 入参/出参
   - `apps/tracer_core/src/api/core_c/`
   - 同步更新：`docs/time_tracer/core/contracts/c_abi.md`
2. 改 Android JNI 能力或回调
   - `apps/tracer_core/src/api/android/native_bridge_calls.cpp`
   - `apps/tracer_core/src/api/android/native_bridge_registration.cpp`
3. 改文件加解密（含进度）
   - `apps/tracer_core/src/infrastructure/crypto/`
   - 同步更新：`docs/time_tracer/core/contracts/crypto/`
4. 改 ingest 流程或校验逻辑
   - `apps/tracer_core/src/application/pipeline/`
   - `apps/tracer_core/src/application/importer/`
   - `apps/tracer_core/src/domain/logic/validator/`
5. 改 query/report 统计结构
   - `apps/tracer_core/src/infrastructure/query/data/`
   - `apps/tracer_core/src/infrastructure/reports/`
   - 同步更新：`docs/time_tracer/core/contracts/stats/`
6. 改数据库读写或 schema
   - `apps/tracer_core/src/infrastructure/persistence/`
   - `apps/tracer_core/src/infrastructure/schema/`

## 4. 跨端同步检查清单（改 core 后必看）
1. 契约文档是否同步（`docs/time_tracer/core/contracts/*`）。
2. Windows CLI 是否需要跟进参数/解析/展示。
3. Android runtime/JNI 是否需要跟进字段与解析。
4. 失败语义是否保持兼容（error code / message / content 字段）。

## 5. Agent 搜索范围建议
优先扫描：
1. `apps/tracer_core/src`
2. `docs/time_tracer/core`
3. `apps/tracer_core/config`（涉及配置契约时）

默认排除：
1. `apps/tracer_core/build/**`
2. `apps/tracer_core/build_fast/**`
3. `apps/tracer_core/build_fast_*/**`
4. `apps/tracer_core/.cache/**`
5. `test/output/**`

参考命令：

```powershell
rg -n --glob '!**/build/**' --glob '!**/build_fast/**' --glob '!**/build_fast_*/**' --glob '!**/.cache/**' "PATTERN" apps/tracer_core docs/time_tracer/core
```

## 6. 最小验证命令
在仓库根目录执行：

```powershell
python scripts/run.py build --app tracer_core --profile fast
python scripts/run.py verify --app tracer_core --profile fast --concise
```

## 7. 常见排错入口
1. ABI 改了但上层异常：先对齐 `c_abi.md` 与 consumer 参数。
2. Android 回调没触发：检查 JNI registration 与 `native_bridge_calls.cpp` 的回调路径。
3. 进度显示异常：检查 `infrastructure/crypto` 快照字段是否完整上报。
4. 统计 JSON 回归：检查 `contracts/stats` 文档与渲染/序列化实现是否一致。
