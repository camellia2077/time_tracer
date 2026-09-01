package com.example.tracer

import androidx.compose.ui.graphics.Color
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class SystemBarAppearanceTest {
    @Test
    fun shouldUseDarkSystemBarIcons_matchesThemeBackgroundLuminance() {
        assertTrue(shouldUseDarkSystemBarIcons(Color.White))
        assertFalse(shouldUseDarkSystemBarIcons(Color.Black))
    }
}
