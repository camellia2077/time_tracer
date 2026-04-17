package com.example.tracer.ui.components

import android.text.InputType
import android.view.inputmethod.EditorInfo
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeMultilineInputConfigTest {
    @Test
    fun inputConfig_prefersLiteralPlainTextEditing_overChatStyleImeFeatures() {
        val config = buildNativeMultilineInputConfig()

        assertTrue(config.inputType and InputType.TYPE_CLASS_TEXT != 0)
        assertTrue(config.inputType and InputType.TYPE_TEXT_FLAG_MULTI_LINE != 0)
        assertTrue(config.inputType and InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS != 0)
        assertTrue(
            config.inputType and InputType.TYPE_MASK_VARIATION ==
                InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD
        )
        assertTrue(config.imeOptions and EditorInfo.IME_FLAG_NO_EXTRACT_UI != 0)
        assertTrue(config.imeOptions and EditorInfo.IME_FLAG_NO_PERSONALIZED_LEARNING != 0)
    }

    @Test
    fun inputConfig_usesStableExpectedBitmask() {
        val config = buildNativeMultilineInputConfig()

        assertEquals(
            InputType.TYPE_CLASS_TEXT or
                InputType.TYPE_TEXT_FLAG_MULTI_LINE or
                InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS or
                InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD,
            config.inputType
        )
        assertEquals(
            EditorInfo.IME_FLAG_NO_EXTRACT_UI or EditorInfo.IME_FLAG_NO_PERSONALIZED_LEARNING,
            config.imeOptions
        )
    }
}
