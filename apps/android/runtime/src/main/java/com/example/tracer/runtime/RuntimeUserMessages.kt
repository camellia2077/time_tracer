package com.example.tracer

import android.content.Context
import com.example.tracer.runtime.R

internal class RuntimeUserMessages(
    private val getString: (Int) -> String = { resourceId ->
        EnglishRuntimeUserMessages.value(resourceId)
    }
) {
    constructor(context: Context) : this(context::getString)

    val changesSaved: String
        get() = getString(R.string.runtime_txt_changes_saved)

    val incompleteDayWarning: String
        get() = getString(R.string.runtime_txt_incomplete_day_warning)

    val overnightContinuationWarning: String
        get() = getString(R.string.runtime_txt_overnight_continuation_warning)
}

private object EnglishRuntimeUserMessages {
    fun value(resourceId: Int): String = when (resourceId) {
        R.string.runtime_txt_changes_saved -> "Changes saved."
        R.string.runtime_txt_incomplete_day_warning ->
            "Some time intervals for this day may not be available yet because it has fewer than two activities."
        R.string.runtime_txt_overnight_continuation_warning ->
            "This day starts with an activity other than waking up, so sleep time may not be calculated automatically."
        else -> error("Unknown runtime user message resource: $resourceId")
    }
}
