package com.example.tracer

import java.security.MessageDigest

private const val HEX_RADIX = 16
private const val UNSIGNED_BYTE_MASK = 0xFF
private const val HEX_PADDING_THRESHOLD = 0x10
private const val HEX_DIGEST_WIDTH = 2

internal fun computeCanonicalTxtSha256Hex(content: String): String {
    val canonicalBytes = CanonicalTextCodec.encodeUtf8(content)
    val digest = MessageDigest.getInstance("SHA-256").digest(canonicalBytes)
    val builder = StringBuilder(digest.size * HEX_DIGEST_WIDTH)
    for (byteValue in digest) {
        val unsigned = byteValue.toInt() and UNSIGNED_BYTE_MASK
        if (unsigned < HEX_PADDING_THRESHOLD) {
            builder.append('0')
        }
        builder.append(unsigned.toString(HEX_RADIX))
    }
    return builder.toString()
}
