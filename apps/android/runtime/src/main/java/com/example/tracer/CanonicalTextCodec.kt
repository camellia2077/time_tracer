package com.example.tracer

import android.content.ContentResolver
import android.net.Uri
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer
import java.nio.charset.CharacterCodingException
import java.nio.charset.CodingErrorAction

class CanonicalTextDecodingException(
    message: String,
    cause: Throwable? = null
) : IllegalArgumentException(message, cause)

object CanonicalTextCodec {
    fun canonicalizeText(text: String): String {
        return text
            .removePrefix("\uFEFF")
            .replace("\r\n", "\n")
            .replace('\r', '\n')
    }

    fun decodeUtf8(
        bytes: ByteArray,
        sourceLabel: String = "text input"
    ): String {
        val decoder = Charsets.UTF_8
            .newDecoder()
            .onMalformedInput(CodingErrorAction.REPORT)
            .onUnmappableCharacter(CodingErrorAction.REPORT)
        val decoded = try {
            decoder.decode(ByteBuffer.wrap(bytes)).toString()
        } catch (error: CharacterCodingException) {
            throw CanonicalTextDecodingException(
                "Invalid UTF-8 in $sourceLabel: decoder rejected the byte sequence.",
                error
            )
        }
        return canonicalizeText(decoded)
    }

    fun encodeUtf8(text: String): ByteArray {
        return canonicalizeText(text).toByteArray(Charsets.UTF_8)
    }

    fun readInputStream(
        input: InputStream,
        sourceLabel: String = "text input"
    ): String {
        return decodeUtf8(input.readBytes(), sourceLabel)
    }

    fun writeOutputStream(
        output: OutputStream,
        content: String
    ) {
        output.write(encodeUtf8(content))
    }

    fun readFile(file: File): String {
        return decodeUtf8(file.readBytes(), file.absolutePath)
    }

    fun writeFile(file: File, content: String) {
        file.writeBytes(encodeUtf8(content))
    }

    fun readUriText(
        contentResolver: ContentResolver,
        uri: Uri,
        sourceLabel: String = uri.toString()
    ): String {
        val input = contentResolver.openInputStream(uri)
            ?: throw IllegalStateException("Cannot open input stream for $uri")
        input.use { stream ->
            return readInputStream(stream, sourceLabel)
        }
    }
}
