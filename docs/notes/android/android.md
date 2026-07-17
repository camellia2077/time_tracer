## release
带 Native 优化的正式 Release APK（不走 QA 流程）
如果 libs/native 那边已经在仓库里构建过，优先使用仓库当前构建好的产物，不需要重新拉取或下载依赖。

python tools/run.py build --app tracer_android --profile android_release


## 无原生优化的 Release APK（不走 QA 流程）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_release_no_opt

## debug版本

python tools/run.py build --app tracer_android --profile android_edit

## 安装 debug APK

adb install -r apps/android/app/build/outputs/apk/debug/app-debug.apk


## adb 打开 app

adb shell am start -n com.example.tracer/.MainActivity


本地快速风格/静态检查，速度更快（ktlint + lintDebug；不含 detekt）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_style
