package com.example.tracer

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.nio.file.Files

class CanonicalTextCodecTest {
    @Test
    fun decodeUtf8_stripsBomAndNormalizesLineEndings() {
        val bytes = byteArrayOf(
            0xEF.toByte(),
            0xBB.toByte(),
            0xBF.toByte(),
            'y'.code.toByte(),
            '2'.code.toByte(),
            '0'.code.toByte(),
            '2'.code.toByte(),
            '6'.code.toByte(),
            '\r'.code.toByte(),
            '\n'.code.toByte(),
            'm'.code.toByte(),
            '0'.code.toByte(),
            '3'.code.toByte(),
            '\r'.code.toByte()
        )

        val decoded = CanonicalTextCodec.decodeUtf8(bytes, "fixture.txt")

        assertEquals("y2026\nm03\n", decoded)
    }

    @Test
    fun decodeUtf8_invalidUtf8Throws() {
        val error = runCatching {
            CanonicalTextCodec.decodeUtf8(byteArrayOf(0xFF.toByte()), "broken.txt")
        }.exceptionOrNull()

        assertTrue(error is CanonicalTextDecodingException)
        assertTrue(error?.message.orEmpty().contains("Invalid UTF-8 in broken.txt"))
    }

    @Test
    fun writeFile_writesUtf8WithoutBomAndLf() {
        val file = Files.createTempFile("canonical-text-codec", ".txt").toFile()
        try {
            CanonicalTextCodec.writeFile(file, "\uFEFFalpha\r\nbeta\r")

            val bytes = file.readBytes()
            assertFalse(
                bytes.size >= 3 &&
                    bytes[0] == 0xEF.toByte() &&
                    bytes[1] == 0xBB.toByte() &&
                    bytes[2] == 0xBF.toByte()
            )
            assertArrayEquals("alpha\nbeta\n".toByteArray(Charsets.UTF_8), bytes)
        } finally {
            file.delete()
        }
    }
}
