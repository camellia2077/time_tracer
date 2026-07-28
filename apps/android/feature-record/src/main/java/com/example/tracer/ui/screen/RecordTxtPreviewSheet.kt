package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.produceState
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import java.time.Clock
import java.time.format.DateTimeFormatter
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private data class TxtPreviewState(
    val isLoading: Boolean,
    val markerResult: TxtDayMarkerResult,
    val dayBlockResult: TxtDayBlockResolveResult?
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun RecordTxtPreviewSheet(
    txtStorageGateway: TxtStorageGateway,
    selectedMonth: String,
    selectedHistoryFile: String,
    editableHistoryContent: String,
    logicalDayTarget: RecordLogicalDayTarget,
    logicalDayClock: Clock,
    isLoading: Boolean,
    previewStatusText: String,
    onDismissRequest: () -> Unit
) {
    val targetDate = resolveLogicalDayTargetDate(
        logicalDayTarget = logicalDayTarget,
        clock = logicalDayClock
    )
    val targetDateIso = targetDate.toString()
    val targetMonth = targetDate.format(DateTimeFormatter.ofPattern("yyyy-MM"))
    val resolvedMonth = selectedMonth.ifBlank { targetMonth }
    val normalizedStatusText = previewStatusText.trim()

    val previewState by produceState(
        initialValue = TxtPreviewState(
            isLoading = true,
            markerResult = TxtDayMarkerResult(
                ok = false,
                normalizedDayMarker = "",
                message = normalizedStatusText
            ),
            dayBlockResult = null
        ),
        selectedHistoryFile,
        editableHistoryContent,
        resolvedMonth,
        targetDateIso,
        isLoading
    ) {
        if (
            isLoading ||
            selectedHistoryFile.isBlank() ||
            resolvedMonth.isBlank()
        ) {
            value = TxtPreviewState(
                isLoading = false,
                markerResult = TxtDayMarkerResult(
                    ok = false,
                    normalizedDayMarker = "",
                    message = normalizedStatusText
                ),
                dayBlockResult = null
            )
            return@produceState
        }

        val markerResult = withContext(Dispatchers.IO) {
            txtStorageGateway.defaultTxtDayMarker(
                selectedMonth = resolvedMonth,
                targetDateIso = targetDateIso
            )
        }
        val dayBlockResult = if (
            markerResult.ok && markerResult.normalizedDayMarker.isNotBlank()
        ) {
            withContext(Dispatchers.IO) {
                txtStorageGateway.resolveTxtDayBlock(
                    content = editableHistoryContent,
                    dayMarker = markerResult.normalizedDayMarker,
                    selectedMonth = resolvedMonth
                )
            }
        } else {
            null
        }

        value = TxtPreviewState(
            isLoading = false,
            markerResult = markerResult,
            dayBlockResult = dayBlockResult
        )
    }
    val markerResult = previewState.markerResult
    val dayBlockResult = previewState.dayBlockResult

    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)

    ModalBottomSheet(
        onDismissRequest = onDismissRequest,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp)
                .verticalScroll(rememberScrollState())
                .testTag(recordTxtPreviewSheetTestTag()),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.record_txt_preview_title),
                style = MaterialTheme.typography.titleLarge,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.record_txt_preview_target_date, targetDateIso),
                style = MaterialTheme.typography.bodyMedium
            )
            Text(
                text = stringResource(R.string.record_txt_preview_target_month, targetMonth),
                style = MaterialTheme.typography.bodyMedium
            )

            when {
                isLoading || previewState.isLoading -> {
                    CircularProgressIndicator(
                        modifier = Modifier.testTag(recordTxtPreviewLoadingTestTag())
                    )
                    Text(
                        text = stringResource(R.string.record_txt_preview_loading),
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                selectedHistoryFile.isBlank() -> {
                    Text(
                        text = normalizedStatusText.ifBlank {
                            stringResource(R.string.record_txt_preview_missing_txt)
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                !markerResult.ok -> {
                    Text(
                        text = markerResult.message.ifBlank {
                            stringResource(R.string.record_txt_preview_marker_failed)
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                dayBlockResult != null && !dayBlockResult.ok -> {
                    Text(
                        text = dayBlockResult.message.ifBlank {
                            stringResource(R.string.record_txt_preview_day_block_failed)
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                dayBlockResult != null && !dayBlockResult.found -> {
                    Text(
                        text = stringResource(R.string.record_txt_preview_day_block_missing),
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                else -> {
                    val resolvedDayBlockResult = requireNotNull(dayBlockResult)
                    Text(
                        text = stringResource(
                            R.string.record_txt_preview_day_marker,
                            markerResult.normalizedDayMarker
                        ),
                        style = MaterialTheme.typography.labelLarge,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Text(
                        text = resolvedDayBlockResult.dayBody.ifBlank {
                            stringResource(R.string.record_txt_preview_day_block_empty)
                        },
                        style = MaterialTheme.typography.bodyLarge,
                        modifier = Modifier.testTag(recordTxtPreviewContentTestTag())
                    )
                }
            }
        }
    }
}

internal fun recordTxtPreviewSheetTestTag(): String = "record_txt_preview_sheet"

internal fun recordTxtPreviewLoadingTestTag(): String = "record_txt_preview_loading"

internal fun recordTxtPreviewContentTestTag(): String = "record_txt_preview_content"
