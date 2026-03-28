本地快速风格/静态检查，速度更快（ktlint + lintDebug；不含 detekt）
程序根目录运行

python tools/run.py build --app tracer_android --profile android_style

python tools/run.py build --app tracer_android --profile android_style *>&1 | Tee-Object docs/android-check.log
