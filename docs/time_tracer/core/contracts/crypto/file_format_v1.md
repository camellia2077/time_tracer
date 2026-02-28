# `.tracer` File Format v1

> 状态：兼容读取保留，当前写入默认升级为 `v2`（见 `file_format_v2.md`）。

## 目标
1. 为导出/分享场景提供跨端可读的加密容器。
2. 支持显式版本化，保证后续升级可兼容处理。

## 总体结构
1. Header（固定 64 字节）
2. Ciphertext（变长，含 AEAD tag）

## Header（64 bytes）
1. `magic` (4 bytes): 固定为 `TTRC`
2. `version` (1 byte): 固定为 `1`
3. `kdf_id` (1 byte): `1` = `Argon2id`
4. `cipher_id` (1 byte): `1` = `XChaCha20-Poly1305-IETF`
5. `reserved` (1 byte): 固定 `0`
6. `ops_limit` (4 bytes, little-endian): KDF 计算强度
7. `mem_limit_kib` (4 bytes, little-endian): KDF 内存参数（KiB）
8. `salt` (16 bytes): KDF salt
9. `nonce` (24 bytes): AEAD nonce
10. `ciphertext_size` (8 bytes, little-endian): 密文区长度（bytes）

## Ciphertext
1. `ciphertext_size` 字节。
2. 内容为 `plaintext + AEAD tag`（由 `crypto_aead_xchacha20poly1305_ietf_encrypt` 产出）。

## 解析规则
1. 文件总长度必须 `>= 64`。
2. `magic != TTRC` 直接拒绝（`unsupported format`）。
3. `version != 1` 直接拒绝（`unsupported version`）。
4. `ciphertext_size` 必须等于 `file_size - 64`。
5. `kdf_id` / `cipher_id` 不支持时必须明确报错，不允许降级尝试。

## 兼容策略
1. `v1` 只定义当前字段，不做可选字段透传。
2. 后续新增格式必须提升 `version`，并保持旧版本显式可识别。
