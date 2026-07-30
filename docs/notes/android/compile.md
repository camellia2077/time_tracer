# Android APK 编译、安装与测试数据注入

本文说明如何把已经构建好的 Debug APK 安装到 Android 设备，并通过 ADB 注入 TXT 和
`activity_hierarchy` 测试数据。

APK 的构建流程见：
`docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`。

Android 构建不需要选择配置数据 profile。构建命令中的 `--profile` 只表示构建流程，
不再使用 `--config-profile`。测试数据不会在编译时打进 APK。

## 1. 确认设备连接

```powershell
adb devices
```

以下命令默认使用唯一连接的设备。如果连接了多台设备，在命令末尾追加
`--serial DEVICE_SERIAL`，其中 `DEVICE_SERIAL` 替换为 `adb devices` 输出的实际序列号。

## 2. 编译与安装命令

从仓库根目录执行。编译和安装是两个独立步骤。
# 只编译
python tools/run.py android --variant debug

python tools/run.py android --variant release

# 编译并安装
python tools/run.py android --variant debug --install

# 编译、安装并注入测试数据
python tools/run.py android --variant debug --install --with-test-data

该命令会在安装后注入 `test/data` 和 `test/data/activity_hierarchy`，清理旧数据库，
并自动启动应用。需要保留已有数据库时增加：

```powershell
--keep-database
```

# 安装已有 APK
python tools/run.py android --variant debug --install-only


### 手动执行单个步骤

如果不需要组合命令，也可以单独安装已经生成的 APK：

```powershell
adb install -r apps/android/app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.example.tracer/.MainActivity
```

## 3. 注入测试数据

测试 TXT 和 `activity_hierarchy` 不会打包进 APK。安装 Debug APK 后，使用以下命令写入应用私有目录：

```powershell
python tools/scripts/devtools/android/push_test_data.py `
  --serial <adb-serial> `
  --launch
```

脚本默认读取：

```text
test/data
test/data/activity_hierarchy
```

并写入：

```text
files/tracer_core/input
files/tracer_core/config/activity_hierarchy
```

脚本默认会停止应用、清理旧 TXT 和数据库，再写入新的测试数据。需要保留已有数据库时，增加：

```powershell
--keep-database
```

注入完成后，脚本可以使用 `--launch` 自动重新启动应用。

## 4. 配置和资源来源

程序自带的只读资源来自：

```text
assets/tracer_core/program
```

Android 构建时会将其同步到 APK 的固定资源路径：

```text
apps/android/runtime/src/main/assets/tracer_core/config
```

默认的可编辑活动分类种子来自：

```text
assets/tracer_core/defaults/activity_hierarchy
```

运行时首次初始化时，会将默认种子复制到应用私有目录：

```text
files/tracer_core/config/activity_hierarchy
```

仓库测试数据使用独立目录：

```text
test/data/activity_hierarchy
```

因此，测试数据应通过 ADB 注入，不应在编译时复制进 APK。

## 5. Data 页的清理范围

Data 页的 **Clear Activity Data** 会删除应用私有目录中的：

- 活动记录 TXT；
- `config/activity_hierarchy` 下的活动分类 TOML；
- 活动数据库及其临时文件。

程序配置资源和其他运行时数据不会被该操作删除。清理活动分类后，应用会在重新初始化时恢复默认 `_system.toml`。

## 6. 相关文档

- Android 构建与验证规范：`docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
- 配置资源生命周期：`docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Android 开发辅助脚本：`tools/scripts/devtools/android/README.md`
