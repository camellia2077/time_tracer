本地快速风格/静态检查，速度更快（ktlint + lintDebug；不含 detekt）
程序根目录运行

python .\scripts\run.py build --app tracer_android --profile android_style

python .\scripts\run.py build --app tracer_android --profile android_style *>&1 | Tee-Object .\scripts\docs\android-check.log



提交前/CI 严格检查，覆盖单测和 release 变体问题，耗时更长
python .\scripts\run.py build --app tracer_android --profile  android_ci *>&1 | Tee-Object .\scripts\docs\android_ci.log


无原生优化的 Release APK（不走 detekt 流程）
python .\scripts\run.py build --app tracer_android --profile android_release_no_opt

