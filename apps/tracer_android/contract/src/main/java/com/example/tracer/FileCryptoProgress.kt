package com.example.tracer

enum class FileCryptoOperation {
    ENCRYPT,
    DECRYPT,
    UNKNOWN;

    companion object {
        fun fromWireValue(value: String): FileCryptoOperation = when (value.lowercase()) {
            "encrypt" -> ENCRYPT
            "decrypt" -> DECRYPT
            else -> UNKNOWN
        }
    }
}

enum class FileCryptoPhase {
    SCAN,
    READ_INPUT,
    COMPRESS,
    DERIVE_KEY,
    ENCRYPT,
    DECRYPT,
    DECOMPRESS,
    WRITE_OUTPUT,
    COMPLETED,
    CANCELLED,
    FAILED,
    UNKNOWN;

    val isTerminal: Boolean
        get() = this == COMPLETED || this == CANCELLED || this == FAILED

    companion object {
        fun fromWireValue(value: String): FileCryptoPhase = when (value.lowercase()) {
            "scan" -> SCAN
            "read_input" -> READ_INPUT
            "compress" -> COMPRESS
            "derive_key" -> DERIVE_KEY
            "encrypt" -> ENCRYPT
            "decrypt" -> DECRYPT
            "decompress" -> DECOMPRESS
            "write_output" -> WRITE_OUTPUT
            "completed" -> COMPLETED
            "cancelled" -> CANCELLED
            "failed" -> FAILED
            else -> UNKNOWN
        }
    }
}

data class FileCryptoProgressEvent(
    val operation: FileCryptoOperation = FileCryptoOperation.UNKNOWN,
    val phase: FileCryptoPhase = FileCryptoPhase.UNKNOWN,
    val currentGroupLabel: String = "",
    val groupIndex: Int = 0,
    val groupCount: Int = 0,
    val fileIndexInGroup: Int = 0,
    val fileCountInGroup: Int = 0,
    val currentFileIndex: Int = 0,
    val totalFiles: Int = 0,
    val currentFileDoneBytes: Long = 0L,
    val currentFileTotalBytes: Long = 0L,
    val overallDoneBytes: Long = 0L,
    val overallTotalBytes: Long = 0L,
    val speedBytesPerSec: Long = 0L,
    val remainingBytes: Long = 0L,
    val etaSeconds: Long = 0L
) {
    val overallProgressFraction: Float
        get() = progressFraction(overallDoneBytes, overallTotalBytes)

    val currentFileProgressFraction: Float
        get() = progressFraction(currentFileDoneBytes, currentFileTotalBytes)

    val overallProgressPercent: Int
        get() = (overallProgressFraction * 100f).toInt()

    val currentFileProgressPercent: Int
        get() = (currentFileProgressFraction * 100f).toInt()

    private fun progressFraction(done: Long, total: Long): Float {
        if (total <= 0L) {
            return if (phase.isTerminal) 1f else 0f
        }
        val boundedDone = done.coerceIn(0L, total)
        return (boundedDone.toDouble() / total.toDouble()).toFloat()
    }
}
