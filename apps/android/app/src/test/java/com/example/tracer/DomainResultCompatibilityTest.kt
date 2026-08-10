package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class DomainResultCompatibilityTest {
    @Test
    fun domainNativeEnvelope_success_maps_to_nativeCallResult() {
        val result = DomainResult.Success(
            DomainNativeEnvelope(
                initialized = true,
                operationOk = true,
                rawResponse = """{"ok":true}""",
                errorLogPath = "/tmp/native.log",
                operationId = "op-native-1"
            )
        ).toLegacyNativeCallResult()

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals("""{"ok":true}""", result.rawResponse)
        assertEquals("/tmp/native.log", result.errorLogPath)
        assertEquals("op-native-1", result.operationId)
    }

    @Test
    fun domainFailure_maps_to_insightsCallResult_with_fallback_payload() {
        val result = DomainResult.Failure(
            CoreError(
                userMessage = "insights failed",
                debugMessage = "native crash",
                errorLogPath = "/tmp/insights.log",
                operationId = "op-insights-1"
            )
        ).toLegacyInsightsCallResult(
            failureRawResponse = { error -> "raw:${error.debugMessage}" },
            failureOutput = { error -> "out:${error.userMessage}" }
        )

        assertFalse(result.initialized)
        assertFalse(result.operationOk)
        assertEquals("out:insights failed", result.outputText)
        assertEquals("raw:native crash", result.rawResponse)
        assertEquals("/tmp/insights.log", result.errorLogPath)
        assertEquals("op-insights-1", result.operationId)
        assertEquals(null, result.errorContract)
        assertEquals(null, result.insightsWindowMetadata)
    }

    @Test
    fun domainNativeEnvelope_insightsFields_map_to_insightsCallResult() {
        val result = DomainResult.Success(
            DomainNativeEnvelope(
                initialized = true,
                operationOk = true,
                rawResponse = """{"ok":true}""",
                outputText = "# Insights",
                operationId = "op-insights-structured",
                errorContract = InsightsErrorContract(
                    errorCode = "insights.target.not_found",
                    errorCategory = "insights",
                    hints = listOf("Try another date.")
                ),
                insightsWindowMetadata = InsightsWindowMetadata(
                    hasRecords = false,
                    matchedDayCount = 0,
                    matchedRecordCount = 0,
                    startDate = "2026-02-01",
                    endDate = "2026-02-07",
                    requestedDays = 7
                )
            )
        ).toLegacyInsightsCallResult()

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals("insights.target.not_found", result.errorContract?.errorCode)
        assertEquals("insights", result.errorContract?.errorCategory)
        assertEquals(listOf("Try another date."), result.errorContract?.hints)
        assertEquals(false, result.insightsWindowMetadata?.hasRecords)
        assertEquals(7, result.insightsWindowMetadata?.requestedDays)
    }

    @Test
    fun domainStringResult_maps_to_legacy_dataQueryTextResult() {
        val success = DomainResult.Success("rows")
            .toLegacyDataQueryTextResult(successMessage = { value -> "loaded:${value.length}" })
        assertTrue(success.ok)
        assertEquals("rows", success.outputText)
        assertEquals("loaded:4", success.message)

        val failure = DomainResult.Failure(
            ValidationError(
                userMessage = "",
                debugMessage = "invalid period"
            )
        ).toLegacyDataQueryTextResult()
        assertFalse(failure.ok)
        assertEquals("", failure.outputText)
        assertEquals("invalid period", failure.message)
    }
}
