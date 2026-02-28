# Clang-Tidy Batch 修复 SOP

本文定义 `time_tracer` 项目处理 clang-tidy 问题的标准作业流程（SOP），目标是统一入口、统一门禁、统一收口，避免手工分叉流程漂移。

## 1. 目标与适用范围

1. 任务队列目录固定为：
   - `apps/tracer_cli/windows/build_tidy/tasks/batch_*/task_*.log`
2. 诊断涉及代码范围：
   - `apps/tracer_cli/windows`
   - `apps/tracer_core`
3. 批次收口统一命令：
   - `python scripts/run.py tidy-batch ...`

## 2. 标准命令（唯一推荐入口）

```bash
python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800
```

其中 `--preset sop` 等价于：
- `--strict-clean --run-verify --concise --full-every 3 --keep-going`

等价 wrapper：

```bash
apps/tracer_cli/windows/scripts/run_clang_tidy.sh <BATCH_ID>
```

## 3. 单批次执行步骤

1. 从 `apps/tracer_cli/windows/build_tidy/tasks/` 选择一个待处理 `batch_xxx`。
2. 逐个处理该批次里的 `task_*.log`（一次只修一个 task，允许同文件聚类 clean）。
3. 每个 task 修复后先做任务级轻量验证：
   - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
   - `task` scope 当前仅包含轻量稳定检查（build + native runtime smoke），不包含 `runtime_guard`。
4. 对同一源码文件的多个 task，建议一次性清理（减少重复 verify）：
   - `python scripts/run.py clean --app tracer_windows_cli --strict --batch-id <BATCH_ID> --cluster-by-file <ID>`
5. 批次修复完成后，执行批次收口命令（批次级全量 verify）：
   - `python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800`
6. 若中断/超时，直接重跑同一命令，自动从 checkpoint 续跑。
7. 确认门禁结果：
   - `test/output/tracer_windows_cli/result.json` 中 `"success": true`
8. 继续下一个批次。

## 4. 严格清理规则（新增）

1. `clean --strict` 不仅要求最新 `result.json` 为 success，还要求：
   - verify 结果时间晚于目标 `task_*.log`；
   - verify 结果时间晚于该 task 对应源码文件更新时间。
2. 条件不满足时，strict clean 会拒绝归档，避免“未验证先清理”。

## 5. 失败分支（仅排障时使用）

仅当 `tidy-batch` 执行失败时，允许临时拆分排查：

```bash
python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task
python scripts/run.py clean --app tracer_windows_cli --strict --batch-id <BATCH_ID> <ID>
python scripts/run.py tidy-refresh --app tracer_windows_cli --batch-id <BATCH_ID> --full-every 3 --keep-going
```

说明：
1. 上述分步命令是排障工具，不是日常主流程。
2. 排障后应回到 `tidy-batch` 主路径。

## 6. 完成标准

1. `apps/tracer_cli/windows/build_tidy/tasks/` 下无 `task_*.log`。
2. `test/output/tracer_windows_cli/result.json` 保持 `"success": true`。
3. 全量收尾建议执行：

```bash
python scripts/run.py tidy-close --app tracer_windows_cli --keep-going --concise
```

## 7. 禁止项

1. 禁止将 `clean + tidy-refresh` 常态化替代 `tidy-batch`。
2. 禁止跳过 verify 门禁直接归档任务（包括绕过 strict 时序校验）。
3. 禁止在非 ABI/FFI 边界使用大范围 `NOLINT` 抑制。
4. 禁止直接修改第三方依赖代码（如 `_deps` 下内容）来“消除” clang-tidy 告警。

