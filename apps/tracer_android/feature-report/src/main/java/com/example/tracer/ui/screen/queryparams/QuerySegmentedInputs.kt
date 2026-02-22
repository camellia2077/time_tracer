package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

private val SegmentYearWidth: Dp = 108.dp
private val SegmentShortWidth: Dp = 68.dp
private val SegmentFieldHeight: Dp = 52.dp

@Composable
internal fun SegmentedDateInput(
    title: String,
    year: String,
    month: String,
    day: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onMonthChange: (String) -> Unit,
    onDayChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-MM-DD)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = month,
            onValueChange = { onMonthChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "MM"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = day,
            onValueChange = { onDayChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "DD"
        )
    }
}

@Composable
internal fun SegmentedYearMonthInput(
    title: String,
    year: String,
    month: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onMonthChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-MM)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = month,
            onValueChange = { onMonthChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "MM"
        )
    }
}

@Composable
internal fun SegmentedYearWeekInput(
    title: String,
    year: String,
    week: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onWeekChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-Www)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-W")
        SegmentNumberField(
            value = week,
            onValueChange = { onWeekChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "WW"
        )
    }
}

@Composable
private fun SegmentNumberField(
    value: String,
    onValueChange: (String) -> Unit,
    keyboardOptions: KeyboardOptions,
    width: Dp,
    placeholder: String
) {
    OutlinedTextField(
        value = value,
        onValueChange = onValueChange,
        placeholder = {
            Text(
                text = placeholder,
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.bodySmall
            )
        },
        singleLine = true,
        keyboardOptions = keyboardOptions,
        textStyle = MaterialTheme.typography.titleMedium.copy(textAlign = TextAlign.Center),
        modifier = Modifier
            .width(width)
            .height(SegmentFieldHeight)
    )
}

@Composable
private fun IsoSeparator(value: String) {
    Text(
        text = value,
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
}
