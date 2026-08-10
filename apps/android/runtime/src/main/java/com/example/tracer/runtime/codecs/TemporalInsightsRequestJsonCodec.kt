package com.example.tracer

import org.json.JSONArray
import org.json.JSONObject

internal class TemporalInsightsRequestJsonCodec {
    fun encodeQuery(request: TemporalInsightsQueryRequest): String {
        val payload = JSONObject()
            .put("operation_kind", InsightsOperationKind.QUERY.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .put("format", request.format.wireValue)
            .put("locale", request.locale)
        appendSelection(payload, request.selection)
        return payload.toString()
    }

    fun encodeStructuredQuery(request: TemporalInsightsQueryRequest): String {
        val payload = JSONObject()
            .put("operation_kind", InsightsOperationKind.STRUCTURED_QUERY.wireValue)
            .put("display_mode", request.displayMode.wireValue)
        appendSelection(payload, request.selection)
        return payload.toString()
    }

    fun encodeTargets(request: TemporalInsightsTargetsRequest): String {
        return JSONObject()
            .put("operation_kind", InsightsOperationKind.TARGETS.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .toString()
    }

    fun encodeExport(request: TemporalInsightsExportRequest): String {
        val payload = JSONObject()
            .put("operation_kind", InsightsOperationKind.EXPORT.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .put("export_scope", request.exportScope.wireValue)
            .put("format", request.format.wireValue)
            .put("locale", request.locale)
        request.selection?.let { appendSelection(payload, it) }
        if (request.recentDaysList.isNotEmpty()) {
            payload.put(
                "recent_days_list",
                JSONArray().apply {
                    request.recentDaysList.forEach(::put)
                }
            )
        }
        return payload.toString()
    }

    private fun appendSelection(
        payload: JSONObject,
        selection: TemporalSelectionPayload
    ) {
        payload.put("selection_kind", selection.kind.wireValue)
        selection.date?.let { payload.put("date", it) }
        selection.startDate?.let { payload.put("start_date", it) }
        selection.endDate?.let { payload.put("end_date", it) }
        selection.days?.let { payload.put("days", it) }
        selection.anchorDate?.let { payload.put("anchor_date", it) }
    }
}
