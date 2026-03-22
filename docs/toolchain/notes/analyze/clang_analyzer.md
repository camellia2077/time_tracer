# clang analyzer

## 目的

独立运行 `clang analyzer`，不混入 `clang-tidy` 的 task 流。

当前已经打通的最小链路是：

1. `analyze`
2. `analyze` 自动 `split`
3. `issue_*.json`
4. `issue_*.toon`
5. `analyze-by-num` 薄 triage

## 常用命令

### 1. 运行 analyzer

```powershell
python tools/run.py analyze --app tracer_core_shell --source-scope core_family --build-dir build_analyze_core_family
```

产物目录：

- `out/analyze/tracer_core_shell/build_analyze_core_family/analyze.log`
- `out/analyze/tracer_core_shell/build_analyze_core_family/reports/run.sarif`
- `out/analyze/tracer_core_shell/build_analyze_core_family/summary.json`
- `out/analyze/tracer_core_shell/build_analyze_core_family/issues/batch_001/issue_001.json`
- `out/analyze/tracer_core_shell/build_analyze_core_family/issues/batch_001/issue_001.toon`

### 2. 单独重拆 issue

```powershell
python tools/run.py analyze-split --app tracer_core_shell --source-scope core_family --build-dir build_analyze_core_family
```

产物目录：

- `out/analyze/tracer_core_shell/build_analyze_core_family/issues/batch_001/issue_001.json`
- `out/analyze/tracer_core_shell/build_analyze_core_family/issues/batch_001/issue_001.toon`

### 3. 按编号读取 issue

工作流入口：

- `.agent/workflows/time_tracer/clang_analyze_by_num.md`

当前语义是：

- `issue selector`
- `artifact resolve`
- `triage guidance`

不包含：

- auto-fix
- auto-close
- issue 自动归档

## 产物分层

### raw

- `run.sarif`

### canonical

- `issue_*.json`

### agent view

- `issue_*.toon`

## 说明

- analyzer 这条线是独立于 `clang-tidy` 的 Python 工作流。
- 默认优先看 `issue_*.toon`，需要稳定字段时再读 `issue_*.json`。
- `analyze` 成功后现在会自动执行一次 `split`。
- 如果你只是想基于已生成的 `run.sarif` 重新拆 issue，才需要单独跑 `analyze-split`。
- 如果代码改动后要重新确认 issue 是否还存在，通常重新跑这一条就够了：

```powershell
python tools/run.py analyze --app tracer_core_shell --source-scope core_family --build-dir build_analyze_core_family
```
