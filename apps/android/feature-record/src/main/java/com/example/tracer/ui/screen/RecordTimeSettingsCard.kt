package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun RecordTimeSettingsCard(
    useManualDate: Boolean,
    manualDate: String,
    currentTimeText: String,
    onUseAutoDate: () -> Unit,
    onUseManualDate: () -> Unit,
    onManualDateChange: (String) -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Row(
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.DateRange,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary
                )
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    Text(
                        text = stringResource(R.string.record_label_target_date),
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Text(
                        text = if (useManualDate) manualDate else currentTimeText.substring(0, 10),
                        style = MaterialTheme.typography.titleMedium
                    )
                }
            }

            Row(verticalAlignment = androidx.compose.ui.Alignment.CenterVertically) {
                Text(
                    text = if (useManualDate) {
                        stringResource(R.string.record_mode_manual)
                    } else {
                        stringResource(R.string.record_mode_auto)
                    },
                    style = MaterialTheme.typography.labelSmall,
                    modifier = Modifier.padding(end = 8.dp)
                )
                Switch(
                    checked = useManualDate,
                    onCheckedChange = { if (it) onUseManualDate() else onUseAutoDate() }
                )
            }
        }

        if (useManualDate) {
            androidx.compose.material3.HorizontalDivider()
            Row(
                modifier = Modifier.padding(16.dp)
            ) {
                com.example.tracer.ui.components.SegmentedDateInput(
                    value = manualDate,
                    onValueChange = onManualDateChange,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}
