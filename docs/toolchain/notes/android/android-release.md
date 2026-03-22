带 Native 优化的正式 Release APK（不走 QA 流程）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_release


无原生优化的 Release APK（不走 QA 流程）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_release_no_opt
