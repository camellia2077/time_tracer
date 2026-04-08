package com.example.tracer

import android.app.AlertDialog
import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import java.time.Instant
import java.time.ZoneId
import java.time.format.DateTimeFormatter
import kotlin.coroutines.resume

private data class TracerExchangeImportPreview(
    val createdAtDisplay: String,
    val packageVersion: String,
    val producerDisplay: String,
    val sourceRootName: String,
    val payloadCountText: String,
)

internal suspend fun confirmTracerExchangeImport(
    context: Context,
    inspectResult: TracerExchangeInspectResult,
): Boolean = withContext(Dispatchers.Main) {
    suspendCancellableCoroutine { continuation ->
        // This dialog is intentionally a lightweight pre-import confirmation
        // step. It surfaces a small package identity summary
        // (time/version/producer/count) instead of the full inspect text so
        // users can quickly catch stale backups before import starts.
        val preview = buildTracerExchangeImportPreview(context, inspectResult)
        val message = context.getString(
            R.string.tracer_import_confirm_message,
            preview.createdAtDisplay,
            preview.packageVersion,
            preview.producerDisplay,
            preview.sourceRootName,
            preview.payloadCountText,
        )

        val dialog = AlertDialog.Builder(context)
            .setTitle(R.string.tracer_import_confirm_title)
            .setMessage(message)
            .setCancelable(true)
            .setNegativeButton(R.string.tracer_import_confirm_cancel) { _, _ ->
                if (continuation.isActive) {
                    continuation.resume(false)
                }
            }
            .setPositiveButton(R.string.tracer_import_confirm_import) { _, _ ->
                if (continuation.isActive) {
                    continuation.resume(true)
                }
            }
            .create()

        dialog.setOnCancelListener {
            if (continuation.isActive) {
                continuation.resume(false)
            }
        }

        continuation.invokeOnCancellation { dialog.dismiss() }
        dialog.show()
    }
}

private fun buildTracerExchangeImportPreview(
    context: Context,
    inspectResult: TracerExchangeInspectResult,
): TracerExchangeImportPreview {
    val createdAtDisplay = formatTracerExchangeCreatedAt(inspectResult.createdAtUtc)
    val packageVersion = if (inspectResult.packageVersion > 0) {
        inspectResult.packageVersion.toString()
    } else {
        context.getString(R.string.tracer_import_confirm_unknown)
    }
    val producerDisplay = buildString {
        val app = inspectResult.producerApp.ifBlank {
            context.getString(R.string.tracer_import_confirm_unknown)
        }
        val platform = inspectResult.producerPlatform.ifBlank {
            context.getString(R.string.tracer_import_confirm_unknown)
        }
        append(app)
        append(" / ")
        append(platform)
    }
    val sourceRootName = inspectResult.sourceRootName.ifBlank {
        context.getString(R.string.tracer_import_confirm_unknown)
    }
    val payloadCountText =
        context.resources.getQuantityString(
            R.plurals.tracer_import_confirm_payload_count,
            inspectResult.payloadFileCount,
            inspectResult.payloadFileCount,
        )
    return TracerExchangeImportPreview(
        createdAtDisplay = createdAtDisplay,
        packageVersion = packageVersion,
        producerDisplay = producerDisplay,
        sourceRootName = sourceRootName,
        payloadCountText = payloadCountText,
    )
}

internal fun formatTracerExchangeCreatedAt(createdAtUtc: String): String {
    if (createdAtUtc.isBlank()) {
        return createdAtUtc
    }
    return runCatching {
        val instant = Instant.parse(createdAtUtc)
        DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm")
            .withZone(ZoneId.systemDefault())
            .format(instant)
    }.getOrElse { createdAtUtc }
}
