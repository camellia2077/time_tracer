package com.example.tracer.ui.components

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
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

private val SegmentYearWidth: Dp = 100.dp
private val SegmentShortWidth: Dp = 64.dp
private val SegmentFieldHeight: Dp = 56.dp

@Composable
fun SegmentedDateInput(
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val (year, month, day) = splitDateIso(value)
    val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)

    Row(
        modifier = modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { nextYear ->
                onValueChange(mergeDateIso(filterDigits(nextYear, 4), month, day))
            },
            keyboardOptions = numericKeyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = month,
            onValueChange = { nextMonth ->
                onValueChange(mergeDateIso(year, filterDigits(nextMonth, 2), day))
            },
            keyboardOptions = numericKeyboardOptions,
            width = SegmentShortWidth,
            placeholder = "MM"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = day,
            onValueChange = { nextDay ->
                onValueChange(mergeDateIso(year, month, filterDigits(nextDay, 2)))
            },
            keyboardOptions = numericKeyboardOptions,
            width = SegmentShortWidth,
            placeholder = "DD"
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
        textStyle = MaterialTheme.typography.bodyLarge.copy(textAlign = TextAlign.Center),
        modifier = Modifier
            .width(width)
            .height(SegmentFieldHeight)
    )
}

@Composable
private fun IsoSeparator(value: String) {
    Text(
        text = value,
        style = MaterialTheme.typography.titleLarge,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
}

private fun filterDigits(value: String, maxLength: Int): String =
    value.filter { it.isDigit() }.take(maxLength)

private fun splitDateIso(value: String): Triple<String, String, String> {
    // Robust split by hyphen for YYYY-MM-DD
    val parts = value.split("-")
    val y = parts.getOrElse(0) { "" }
    val m = parts.getOrElse(1) { "" }
    val d = parts.getOrElse(2) { "" }
    return Triple(y, m, d)
}

private fun mergeDateIso(year: String, month: String, day: String): String {
    return "$year-$month-$day"
}
