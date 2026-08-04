package com.example.tracer
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import android.util.Log
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.CalendarAvailability
import java.time.Clock
import java.time.LocalDate
import java.time.format.DateTimeFormatter
import java.time.format.DateTimeParseException
import kotlinx.coroutines.launch


@Composable
internal fun TxtSelectionHintCard() {
    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_unselected_state_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.txt_unselected_state_description),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

// Empty-state guidance card for users who have no TXT files yet.
// This is the primary entry point for fresh release installs where no
// bundled test data exists. Creating the current month TXT bootstraps
// the file with mandatory header lines (yYYYY, mMM) so that the
// Record Input flow can immediately append day blocks on demand.
@Composable
internal fun TxtEmptyStateCard(onCreateCurrentMonthTxt: () -> Unit) {
    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_empty_state_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.txt_empty_state_description),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Button(
                onClick = onCreateCurrentMonthTxt,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.txt_action_create_current_month))
            }
        }
    }
}

internal data class YearMonthKey(
    val year: String,
    val month: String
) {
    val key: String
        get() = "$year-$month"
}

internal fun parseYearMonthKey(value: String): YearMonthKey? {
    val normalized = value.trim()
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(normalized)
        ?: return null
    val year = match.groupValues[1]
    val month = match.groupValues[2]
    val monthInt = month.toIntOrNull() ?: return null
    if (monthInt !in 1..12) {
        return null
    }
    return YearMonthKey(year = year, month = month)
}

internal fun resolveDisplayedCurrentDay(
    selectedMonth: String,
    normalizedDayMarker: String,
    resolvedIsoDate: String?
): LocalDate? {
    parseIsoDateOrNull(resolvedIsoDate)?.let { return it }
    val yearMonth = parseYearMonthKey(selectedMonth) ?: return null
    if (normalizedDayMarker.length != 4) {
        return null
    }
    val month = normalizedDayMarker.take(2).toIntOrNull() ?: return null
    val day = normalizedDayMarker.drop(2).toIntOrNull() ?: return null
    return try {
        LocalDate.of(yearMonth.year.toInt(), month, day)
    } catch (_: RuntimeException) {
        null
    }
}

internal fun parseIsoDateOrNull(value: String?): LocalDate? {
    if (value.isNullOrBlank()) {
        return null
    }
    return try {
        LocalDate.parse(value)
    } catch (_: DateTimeParseException) {
        null
    }
}

internal fun navigateToAdjacentDay(
    currentDay: LocalDate?,
    dayOffset: Long,
    selectedMonth: String,
    onPendingDayChange: (LocalDate?) -> Unit,
    onDayMarkerInputChange: (String) -> Unit,
    onOpenMonth: (String) -> Unit
) {
    val baseDay = currentDay ?: return
    navigateToDay(
        targetDay = baseDay.plusDays(dayOffset),
        selectedMonth = selectedMonth,
        onPendingDayChange = onPendingDayChange,
        onDayMarkerInputChange = onDayMarkerInputChange,
        onOpenMonth = onOpenMonth
    )
}

internal fun navigateToDay(
    targetDay: LocalDate,
    selectedMonth: String,
    onPendingDayChange: (LocalDate?) -> Unit,
    onDayMarkerInputChange: (String) -> Unit,
    onOpenMonth: (String) -> Unit
) {
    onDayMarkerInputChange(formatDayMarker(targetDay))
    val targetMonth = formatMonthKey(targetDay)
    if (targetMonth == selectedMonth) {
        onPendingDayChange(null)
        return
    }
    onPendingDayChange(targetDay)
    onOpenMonth(targetMonth)
}

internal fun formatMonthKey(date: LocalDate): String =
    date.format(DateTimeFormatter.ofPattern("yyyy-MM"))

internal fun formatDayMarker(date: LocalDate): String =
    date.format(DateTimeFormatter.ofPattern("MMdd"))

