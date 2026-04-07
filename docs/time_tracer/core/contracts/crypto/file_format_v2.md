# `.tracer` File Format v2 (outer crypto carrier)

## 目标
1. 在保持跨端加密互通的前提下，减少导出文件体积。
2. 明确“先压缩后加密”的固定处理顺序与字段约束。
3. 只定义外层 `.tracer` 二进制容器；不负责定义解密后明文 payload 的业务布局。

## 总体结构
1. Header（固定 80 字节）
2. Ciphertext（变长，内容是压缩数据经 AEAD 加密后的密文）

## Header（80 bytes）
1. `magic` (4 bytes): 固定 `TTRC`
2. `version` (1 byte): 固定 `2`
3. `kdf_id` (1 byte): `1` = `Argon2id`
4. `cipher_id` (1 byte): `1` = `XChaCha20-Poly1305-IETF`
5. `compression_id` (1 byte): `1` = `zstd`
6. `compression_level` (1 byte): 本轮固定 `1`
7. `reserved_a` (3 bytes): 固定 `0`
8. `ops_limit` (4 bytes, little-endian): KDF 计算强度
9. `mem_limit_kib` (4 bytes, little-endian): KDF 内存参数（KiB）
10. `salt` (16 bytes): KDF salt
11. `nonce` (24 bytes): AEAD nonce
12. `plaintext_size` (8 bytes, little-endian): 解压后的明文长度
13. `ciphertext_size` (8 bytes, little-endian): 密文区长度
14. `reserved_b` (4 bytes): 固定 `0`

## 数据链路
1. 外层编码固定为：`plaintext payload -> zstd(level=1) -> encrypt(XChaCha20-Poly1305) -> .tracer`
2. 外层解码固定为：`.tracer -> decrypt -> zstd decompress -> plaintext payload`
3. 当前 Windows tracer exchange 流程中，`plaintext payload` 是 `tracer_exchange_package_v4` 定义的 tracer exchange 包字节流：
   - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`

## 解析规则
1. 文件总长度必须 `>= 80`。
2. `magic != TTRC` 直接拒绝（`unsupported format`）。
3. `version != 2` 按对应版本处理或拒绝。
4. `reserved_a` 与 `reserved_b` 必须全为 `0`。
5. `compression_id` 目前只支持 `1(zstd)`。
6. `ciphertext_size` 必须等于 `file_size - 80`。
7. 解压后长度必须严格等于 `plaintext_size`，否则报元信息不匹配错误。

## 载荷解释约束
1. `file_format_v2` 只约束外层 `.tracer` 容器，不单独保证业务可导入。
2. 当前 tracer exchange `decrypt/import` / `inspect` 路径除了要求外层 `v2` 合法，还要求解密后的明文 payload 必须满足：
   - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
3. 历史上“外层 v2 合法、但明文 payload 不是当前支持的 tracer exchange package”的文件，不属于当前 exchange import 成功范围。

## 与 v1 兼容策略
1. 外层 header 解码器仍可支持：
   - `v1`（无压缩元信息，按历史路径处理）
   - `v2`（压缩后加密）
2. 编码器默认输出 `v2`。
3. 后续新增外层容器格式继续升级 `version`，禁止 silent break。
