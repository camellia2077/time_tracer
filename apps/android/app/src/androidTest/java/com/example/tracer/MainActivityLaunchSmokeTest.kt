package com.example.tracer

import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertFalse
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class MainActivityLaunchSmokeTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<MainActivity>()

    @Test
    fun appLaunchesWithoutImmediateCrash() {
        composeRule.waitForIdle()
        assertFalse(
            "MainActivity should remain alive after the first frame is rendered.",
            composeRule.activity.isFinishing
        )
    }
}
