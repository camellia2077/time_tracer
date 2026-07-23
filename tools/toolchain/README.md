# Toolchain Index

本目录只保留代码目录说明；命令、流程和结果契约统一维护在 `docs/tools/toolchain/`。

## 代码结构

1. `cli/`
   - 参数注册与 handler
2. `commands/`
   - 构建、验证、clang-tidy 等执行逻辑
3. `config/`
   - profile 与 workflow 配置
4. `core/`
   - 上下文、配置、进程执行等基础设施
5. `services/`
   - 纯逻辑服务与状态辅助
6. `formats/`
   - TOON 等内部格式

## 文档入口

1. `../../docs/tools/toolchain/README.md`
2. `../../docs/tools/toolchain/tools/README.md`
3. `../../docs/tools/toolchain/test/README.md`
4. `../../docs/tools/toolchain/command_map/README.md`
5. `../../docs/tools/toolchain/tidy/README.md`
6. `../../docs/tools/toolchain/workflows/README.md`
7. `../../docs/tools/toolchain/history/README.md`
8. `../AGENTS.md`
