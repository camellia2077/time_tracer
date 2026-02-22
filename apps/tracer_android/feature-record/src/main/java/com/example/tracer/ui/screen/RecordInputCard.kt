package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun RecordInputCard(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    onRecordNow: () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.record_title_record_input),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            OutlinedTextField(
                value = recordContent,
                onValueChange = onRecordContentChange,
                label = { Text(stringResource(R.string.record_label_activity_name)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = true,
                shape = MaterialTheme.shapes.large,
                modifier = Modifier.fillMaxWidth()
            )

            OutlinedTextField(
                value = recordRemark,
                onValueChange = onRecordRemarkChange,
                label = { Text(stringResource(R.string.record_label_remark_optional)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = true,
                shape = MaterialTheme.shapes.large,
                modifier = Modifier.fillMaxWidth()
            )

            Button(
                onClick = onRecordNow,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
            ) {
                Icon(Icons.Default.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.record_action_record_activity))
            }
        }
    }
}
