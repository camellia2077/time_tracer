package com.example.tracer

import org.json.JSONArray
import org.json.JSONObject

internal class TemporalReportRequestJsonCodec {
    fun encodeQuery(request: TemporalReportQueryRequest): String {
        val payload = JSONObject()
            .put("operation_kind", ReportOperationKind.QUERY.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .put("format", request.format.wireValue)
        appendSelection(payload, request.selection)
        return payload.toString()
    }

    fun encodeTargets(request: TemporalReportTargetsRequest): String {
        return JSONObject()
            .put("operation_kind", ReportOperationKind.TARGETS.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .toString()
    }

    fun encodeExport(request: TemporalReportExportRequest): String {
        val payload = JSONObject()
            .put("operation_kind", ReportOperationKind.EXPORT.wireValue)
            .put("display_mode", request.displayMode.wireValue)
            .put("export_scope", request.exportScope.wireValue)
            .put("format", request.format.wireValue)
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
