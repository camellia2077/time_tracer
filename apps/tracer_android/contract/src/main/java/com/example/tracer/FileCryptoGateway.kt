package com.example.tracer

interface FileCryptoGateway {
    suspend fun encryptTxtFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): RecordActionResult = RecordActionResult(
        ok = false,
        message = "Encrypt TXT is not supported by current runtime."
    )

    suspend fun decryptTracerFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): RecordActionResult = RecordActionResult(
        ok = false,
        message = "Decrypt .tracer is not supported by current runtime."
    )
}
