## release
带 Native 优化的正式 Release APK（不走 QA 流程）
如果 libs/native 那边已经在仓库里构建过，优先使用仓库当前构建好的产物，不需要重新拉取或下载依赖。

python tools/run.py build --app tracer_android --profile android_release --config-profile distribution


## 无原生优化的 Release APK（不走 QA 流程）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_release_no_opt --config-profile distribution


## debug版本
编译debug并安装到手机

```bash
python tools/run.py build --app tracer_android --profile android_edit_device --config-profile test
```
编译安装包

python tools/run.py build --app tracer_android --profile android_edit --config-profile test






编译 Android APK 时必须显式指定 `--config-profile`。打包给新用户时使用
`distribution` 配置：

```bash
python tools/run.py build --app tracer_android --profile android_release --config-profile distribution
```

也可以显式选择测试配置：

```bash
python tools/run.py build --app tracer_android --profile android_edit --config-profile test
```

省略 `--config-profile` 的 Android 编译命令会直接失败，不会启动 Gradle。

配置来源位于：

```text
assets/tracer_core/config_distribution
assets/tracer_core/config_test
```

构建时会将所选目录同步到 Android 固定运行路径：

```text
apps/android/runtime/src/main/assets/tracer_core/config
```

因此 Core 和 Android 的配置解析路径不随构建配置改变。`distribution` 只包含适合新用户的空白 alias 配置，`test` 包含与 `test/data` 配套、用于查看 report 的测试 alias 配置。

## 安装 debug APK

adb install -r apps/android/app/build/outputs/apk/debug/app-debug.apk


## adb 打开 app

adb shell am start -n com.example.tracer/.MainActivity


本地快速风格/静态检查，速度更快（ktlint + lintDebug；不含 detekt）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_style --config-profile test
