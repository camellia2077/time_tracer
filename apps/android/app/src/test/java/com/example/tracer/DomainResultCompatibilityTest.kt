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
    fun domainFailure_maps_to_reportCallResult_with_fallback_payload() {
        val result = DomainResult.Failure(
            CoreError(
                userMessage = "report failed",
                debugMessage = "native crash",
                errorLogPath = "/tmp/report.log",
                operationId = "op-report-1"
            )
        ).toLegacyReportCallResult(
            failureRawResponse = { error -> "raw:${error.debugMessage}" },
            failureOutput = { error -> "out:${error.userMessage}" }
        )

        assertFalse(result.initialized)
        assertFalse(result.operationOk)
        assertEquals("out:report failed", result.outputText)
        assertEquals("raw:native crash", result.rawResponse)
        assertEquals("/tmp/report.log", result.errorLogPath)
        assertEquals("op-report-1", result.operationId)
        assertEquals(null, result.errorContract)
        assertEquals(null, result.reportWindowMetadata)
    }

    @Test
    fun domainNativeEnvelope_reportFields_map_to_reportCallResult() {
        val result = DomainResult.Success(
            DomainNativeEnvelope(
                initialized = true,
                operationOk = true,
                rawResponse = """{"ok":true}""",
                outputText = "# Report",
                operationId = "op-report-structured",
                errorContract = ReportErrorContract(
                    errorCode = "reporting.target.not_found",
                    errorCategory = "reporting",
                    hints = listOf("Try another date.")
                ),
                reportWindowMetadata = ReportWindowMetadata(
                    hasRecords = false,
                    matchedDayCount = 0,
                    matchedRecordCount = 0,
                    startDate = "2026-02-01",
                    endDate = "2026-02-07",
                    requestedDays = 7
                )
            )
        ).toLegacyReportCallResult()

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals("reporting.target.not_found", result.errorContract?.errorCode)
        assertEquals("reporting", result.errorContract?.errorCategory)
        assertEquals(listOf("Try another date."), result.errorContract?.hints)
        assertEquals(false, result.reportWindowMetadata?.hasRecords)
        assertEquals(7, result.reportWindowMetadata?.requestedDays)
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
