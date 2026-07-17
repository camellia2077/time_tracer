package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.ListItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.CalendarAvailability

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun ReportYearPickerInput(
    title: String,
    reportYear: String,
    availability: CalendarAvailability,
    onReportYearChange: (String) -> Unit
) {
    var visible by remember { mutableStateOf(false) }
    val years = availability.years

    Text(
        text = title,
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    OutlinedButton(
        onClick = { visible = true },
        enabled = years.isNotEmpty(),
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(
            text = reportYear.ifBlank {
                stringResource(R.string.report_picker_year_placeholder)
            },
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
    }

    if (visible) {
        val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
        ModalBottomSheet(
            onDismissRequest = { visible = false },
            sheetState = sheetState
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    text = stringResource(R.string.report_sheet_select_year),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                LazyColumn(modifier = Modifier.fillMaxWidth()) {
                    items(years) { year ->
                        ListItem(
                            headlineContent = { Text(year) },
                            modifier = Modifier.clickable {
                                onReportYearChange(year)
                                visible = false
                            }
                        )
                    }
                }
            }
        }
    }
}
