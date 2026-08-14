package com.example.tracer

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class UnconfiguredActivityRecordPromptTest {
    @Test
    fun prompt_isShown_onlyWhenEnabledAndActivityIsNotAuthorable() {
        assertTrue(
            shouldConfirmUnconfiguredActivityRecord(
                activityName = "new activity",
                promptEnabled = true,
                validAuthorableEventTokens = setOf("configured")
            )
        )
        assertFalse(
            shouldConfirmUnconfiguredActivityRecord(
                activityName = " configured ",
                promptEnabled = true,
                validAuthorableEventTokens = setOf("configured")
            )
        )
        assertFalse(
            shouldConfirmUnconfiguredActivityRecord(
                activityName = "new activity",
                promptEnabled = false,
                validAuthorableEventTokens = emptySet()
            )
        )
    }
}
