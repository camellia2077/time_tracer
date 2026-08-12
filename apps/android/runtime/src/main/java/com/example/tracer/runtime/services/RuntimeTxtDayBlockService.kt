package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import org.json.JSONArray

// This bridge keeps Android out of the day-block business logic by translating
// UI-facing requests into the shared runtime txt actions.
internal class RuntimeTxtDayBlockService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val nativeTxt: (String) -> String,
    private val codec: NativeTxtRuntimeCodec
) {
    suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtDayMarkerResult(
                ok = false,
                normalizedDayMarker = "",
                message = "native init failed."
            )
        }

        try {
            codec.parseDayMarker(
                nativeTxt(
                    JSONObject()
                        .put("action", "default_day_marker")
                        .put("selected_month", selectedMonth)
                        .put("target_date_iso", targetDateIso)
                        .toString()
                )
            )
        } catch (error: Exception) {
            TxtDayMarkerResult(
                ok = false,
                normalizedDayMarker = "",
                message = formatNativeFailure("default txt day marker failed", error)
            )
        }
    }

    suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = dayMarker,
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = "native init failed."
            )
        }

        try {
            codec.parseResolve(
                nativeTxt(
                    JSONObject()
                        .put("action", "resolve_day_block")
                        .put("content", content)
                        .put("day_marker", dayMarker)
                        .put("selected_month", selectedMonth)
                        .toString()
                )
            )
        } catch (error: Exception) {
            TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = dayMarker,
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = formatNativeFailure("resolve txt day block failed", error)
            )
        }
    }

    suspend fun replaceTxtDayBlock(
        content: String,
        dayMarker: String,
        editedDayBody: String
    ): TxtDayBlockReplaceResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtDayBlockReplaceResult(
                ok = false,
                normalizedDayMarker = dayMarker,
                found = false,
                isMarkerValid = false,
                updatedContent = content,
                message = "native init failed."
            )
        }

        try {
            codec.parseReplace(
                nativeTxt(
                    JSONObject()
                        .put("action", "replace_day_block")
                        .put("content", content)
                        .put("day_marker", dayMarker)
                        .put("edited_day_body", editedDayBody)
                        .toString()
                )
            )
        } catch (error: Exception) {
            TxtDayBlockReplaceResult(
                ok = false,
                normalizedDayMarker = dayMarker,
                found = false,
                isMarkerValid = false,
                updatedContent = content,
                message = formatNativeFailure("replace txt day block failed", error)
            )
        }
    }

    suspend fun resolveTxtDayEdit(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayEditResolveResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtDayEditResolveResult(
                ok = false, normalizedDayMarker = dayMarker, found = false,
                isMarkerValid = false, canSave = false, dayRemark = "", events = emptyList(),
                dayContentIsoDate = null, message = "native init failed."
            )
        }
        try {
            codec.parseDayEditResolve(
                nativeTxt(
                    JSONObject()
                        .put("action", "resolve_day_edit")
                        .put("content", content)
                        .put("day_marker", dayMarker)
                        .put("selected_month", selectedMonth)
                        .toString()
                )
            )
        } catch (error: Exception) {
            TxtDayEditResolveResult(
                ok = false, normalizedDayMarker = dayMarker, found = false,
                isMarkerValid = false, canSave = false, dayRemark = "", events = emptyList(),
                dayContentIsoDate = null,
                message = formatNativeFailure("resolve txt structured day edit failed", error)
            )
        }
    }

    suspend fun applyTxtDayEdit(
        content: String,
        dayMarker: String,
        selectedMonth: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>
    ): TxtDayEditApplyResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtDayEditApplyResult(
                ok = false, normalizedDayMarker = dayMarker, found = false,
                isMarkerValid = false, updatedContent = content, message = "native init failed."
            )
        }
        try {
            val eventsJson = JSONArray()
            events.forEach { event ->
                eventsJson.put(
                    JSONObject()
                        .put("is_interval", event.isInterval)
                        .put("start_time", event.startTime)
                        .put("end_time", event.endTime)
                        .put("activity_token", event.activityToken)
                        .put("remark", event.remark)
                )
            }
            codec.parseDayEditApply(
                nativeTxt(
                    JSONObject()
                        .put("action", "apply_day_edit")
                        .put("content", content)
                        .put("day_marker", dayMarker)
                        .put("selected_month", selectedMonth)
                        .put("day_remark", dayRemark)
                        .put("events", eventsJson)
                        .toString()
                )
            )
        } catch (error: Exception) {
            TxtDayEditApplyResult(
                ok = false, normalizedDayMarker = dayMarker, found = false,
                isMarkerValid = false, updatedContent = content,
                message = formatNativeFailure("apply txt structured day edit failed", error)
            )
        }
    }
}
