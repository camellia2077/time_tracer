package com.example.tracer

import java.security.MessageDigest

internal fun computeCanonicalTxtSha256Hex(content: String): String {
    val canonicalBytes = CanonicalTextCodec.encodeUtf8(content)
    val digest = MessageDigest.getInstance("SHA-256").digest(canonicalBytes)
    val builder = StringBuilder(digest.size * 2)
    for (byteValue in digest) {
        val unsigned = byteValue.toInt() and 0xFF
        if (unsigned < 0x10) {
            builder.append('0')
        }
        builder.append(unsigned.toString(16))
    }
    return builder.toString()
}
