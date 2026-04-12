# Android Devtools

本目录存放 Android 相关开发辅助脚本，不参与默认 `build/verify/validate` 流水线。

## 脚本

1. `setup_android_release_signing.py`
   - 生成正式 Android release keystore。
   - 可选写入 `apps/android/keystore.properties`。
   - 默认拒绝覆盖已有 keystore，避免误替换正式签名。
2. `sync_android_input_from_test_data.py`
   - 同步 Android debug 运行时测试输入。

## setup_android_release_signing.py

### 目的

把 Android release 签名初始化这件事做成可重复执行的脚本，减少手工操作出错的概率。

适用场景：

- 项目还没有正式 release keystore，需要生成第一份正式签名。
- 你想在新电脑上快速恢复同一套本地签名配置。
- 你想统一生成 `apps/android/keystore.properties`。

不适用场景：

- 已经发布过正式 APK，但你打算“重新生成另一把 key”继续覆盖安装旧版本。
  - 这种情况下会导致签名变化，旧安装包通常无法直接升级覆盖。

### 默认行为

- 默认 keystore 输出路径：
  - `apps/android/app/tracer-release.jks`
- 默认 alias：
  - `tracer-release`
- 默认有效期：
  - 25 年
- 默认不会覆盖已有 keystore。
- 默认不会覆盖已有 `apps/android/keystore.properties`。

### 先看计划，不落盘

推荐第一次先执行 dry-run：

```powershell
python tools/scripts/devtools/android/setup_android_release_signing.py `
  --generate-passwords `
  --write-keystore-properties `
  --overwrite-keystore-properties `
  --dry-run
```

这个命令会：

- 预览 `keytool` 调用参数
- 预览要写入的 `keystore.properties`
- 打印生成后的密码摘要
- 不真正创建文件

### 自动生成密码并写本地配置

如果你接受脚本生成强密码，可以直接执行：

```powershell
python tools/scripts/devtools/android/setup_android_release_signing.py `
  --generate-passwords `
  --write-keystore-properties `
  --overwrite-keystore-properties
```

执行后会：

- 生成 `apps/android/app/tracer-release.jks`
- 写入 `apps/android/keystore.properties`
- 在终端打印：
  - `STORE_PASSWORD`
  - `KEY_ALIAS`
  - `KEY_PASSWORD`

这些值必须马上保存到安全位置。

### 手动指定密码

如果你希望自己决定密码，可以这样执行：

```powershell
python tools/scripts/devtools/android/setup_android_release_signing.py `
  --store-password "your-strong-store-password" `
  --key-password "your-strong-key-password" `
  --write-keystore-properties `
  --overwrite-keystore-properties
```

如果你不传 `--key-password`，脚本会默认让 `KEY_PASSWORD` 等于 `STORE_PASSWORD`。

### 自定义 keystore 路径

如果你不想把正式 keystore 放在仓库里，可以指定仓库外路径：

```powershell
python tools/scripts/devtools/android/setup_android_release_signing.py `
  --keystore-path "C:\secure\android-signing\tracer-release.jks" `
  --store-password "your-strong-store-password" `
  --key-password "your-strong-key-password" `
  --write-keystore-properties `
  --overwrite-keystore-properties
```

这通常比把 `.jks` 放在仓库目录里更安全。

### 使用环境变量提供密码

如果你不想把密码直接写进命令行，可以先设置环境变量：

```powershell
$env:TT_ANDROID_RELEASE_STORE_PASSWORD="your-strong-store-password"
$env:TT_ANDROID_RELEASE_KEY_PASSWORD="your-strong-key-password"

python tools/scripts/devtools/android/setup_android_release_signing.py `
  --write-keystore-properties `
  --overwrite-keystore-properties
```

### 保护机制

脚本默认很保守：

- 如果 keystore 文件已存在，不会覆盖。
- 如果 `keystore.properties` 已存在，不会覆盖。

如果你明确知道自己在做什么，可以使用：

- `--force`
  - 允许覆盖已有 keystore 文件。
- `--overwrite-keystore-properties`
  - 允许覆盖已有 `keystore.properties`。

注意：

- 不要对已经用于正式发布的 keystore 随意使用 `--force`。

### 生成完成后建议做的事

1. 备份以下信息到安全位置：
   - `.jks` 文件
   - `STORE_PASSWORD`
   - `KEY_ALIAS`
   - `KEY_PASSWORD`
2. 确认 `.jks` 文件没有被误提交到 Git。
3. 跑一次正式构建确认签名配置可用：

```powershell
python tools/run.py build --app tracer_android --profile android_release
```

### 在其他电脑上复用同一签名

如果要保证不同电脑编译出的 release APK 签名一致，需要复用同一套资料：

- 同一个 `.jks` 文件
- 同一个 `STORE_PASSWORD`
- 同一个 `KEY_ALIAS`
- 同一个 `KEY_PASSWORD`

最常见做法是：

1. 把 `.jks` 安全复制到新电脑
2. 执行脚本并指定相同的 `--keystore-path`
3. 用同一套密码生成本地 `keystore.properties`

### 风险提醒

如果旧版本 APK 已经对外安装过：

- 不要重新生成一把新的 release keystore 再继续发布。
- 否则新 APK 会因为签名不同，无法直接覆盖安装旧版本。
