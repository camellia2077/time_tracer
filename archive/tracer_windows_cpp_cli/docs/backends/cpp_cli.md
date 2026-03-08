# Windows CLI Backend - C++ (`tracer_windows_cli`)

## 角色
1. Windows CLI 集成交付主轨（当前 `tracer_windows_cli` 套件执行目标）。
2. 承担 core runtime 适配、命令分发与主要展现层实现。

## 代码边界
1. 根目录：`archive/tracer_windows_cpp_cli/`
2. 入口与 app 编排：`archive/tracer_windows_cpp_cli/src/api/cli/impl/app/`
3. 命令实现：`archive/tracer_windows_cpp_cli/src/api/cli/impl/commands/`
4. 表现层：`archive/tracer_windows_cpp_cli/src/api/cli/impl/presentation/`
5. bootstrap/runtime 工厂：`archive/tracer_windows_cpp_cli/src/bootstrap/`

## 构建与验证
1. 推荐日常单命令验证（包含构建 + 套件）：  
   `python tools/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise`
2. 里程碑/发版前验证：  
   `python tools/run.py verify --app tracer_core --quick --scope batch --concise`
3. C++ 轨显式构建：  
   `python tools/run.py configure --app tracer_windows_cli --build-dir build_fast`  
   `python tools/run.py build --app tracer_windows_cli --build-dir build_fast`
4. C++ 轨 artifact 验证（显式）：  
   `python tools/run.py verify --app tracer_core --build-dir build_fast --scope artifact --concise`

## 当前实现差异（相对 Rust）
1. `tracer_windows_cli` 套件直接覆盖该 backend。
2. `--license` 与 `licenses [--full]` 由 C++ 顶层信息命令分支处理（无需先构建 runtime）。
3. 该 backend 作为 Rust 轨部分命令的委托目标（例如 `crypto`）。

## 接口边界（Phase 6 收口）
1. 报表 formatter 实现归属 `apps/tracer_core/src/infrastructure/reports/`（进程内 `reports_shared`），不再暴露插件 ABI。
2. C++ CLI 仅依赖 `tracer_core` 暴露的 runtime/C API 能力，不直接依赖 formatter 内部类型。
3. 对外命令契约以 CLI 参数/输出为准，formatter 内部实现细节不进入 CLI 公共接口。

