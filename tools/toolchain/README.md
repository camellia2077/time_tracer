# Toolchain Index

本目录只保留代码目录说明；命令、流程和结果契约统一维护在 `docs/toolchain/`。

## 代码结构

1. `cli/`
   - 参数注册与 handler
2. `commands/`
   - 构建、验证、clang-tidy、validate 等执行逻辑
3. `config/`
   - profile 与 workflow 配置
4. `core/`
   - 上下文、配置、进程执行等基础设施
5. `services/`
   - 纯逻辑服务与状态辅助
6. `formats/`
   - TOON 等内部格式

## 文档入口

1. `../../docs/toolchain/README.md`
2. `../../docs/toolchain/tools/README.md`
3. `../../docs/toolchain/test/README.md`
4. `../../docs/toolchain/command_map/README.md`
5. `../../docs/toolchain/tidy/README.md`
6. `../../docs/toolchain/validate/README.md`
7. `../../docs/toolchain/workflows/README.md`
8. `../../docs/toolchain/notes/README.md`
9. `../../docs/toolchain/history/README.md`
10. `../AGENTS.md`
