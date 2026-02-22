Role (角色): Backend Synchronizer & Linux Headless Client (后端同步中心与 Linux 无界面客户端)。

Tech Stack (技术栈): Rust (Tokio / Axum / Serde)。

Key Features (核心功能):

Network IO: 实现基于 HTTP/gRPC 的数据同步协议，接收来自 tracer_android_gui 的数据。

Linux Implementation: 适配 Linux 环境的文件系统和守护进程（Daemon）逻辑。

Persistence: 调用 time_tracer_core 进行跨平台数据入库。