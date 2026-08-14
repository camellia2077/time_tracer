package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.Row
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.QuestionMark
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.data.AppLanguage

@Composable
internal fun ConfigApplicationPreferencesCard(
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit,
    promptBeforeUnconfiguredActivityRecord: Boolean,
    onPromptBeforeUnconfiguredActivityRecordChange: (Boolean) -> Unit,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {}
) {
    var showUnconfiguredActivityDescription by rememberSaveable { mutableStateOf(false) }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp)
        ) {
            ConfigCardHeader(
                title = stringResource(R.string.config_title_app_preferences),
                expanded = expanded,
                onToggleExpanded = onToggleExpanded
            )
            if (expanded) {
                LanguageSection(
                    appLanguage = appLanguage,
                    onSetAppLanguage = onSetAppLanguage
                )
                HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp))
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Row(verticalAlignment = androidx.compose.ui.Alignment.CenterVertically) {
                            Text(
                                text = stringResource(R.string.config_label_prompt_unconfigured_activity),
                                style = MaterialTheme.typography.bodyMedium,
                                modifier = Modifier.weight(1f)
                            )
                            IconButton(
                                onClick = {
                                    showUnconfiguredActivityDescription =
                                        !showUnconfiguredActivityDescription
                                }
                            ) {
                                Icon(
                                    imageVector = Icons.Filled.QuestionMark,
                                    contentDescription = stringResource(
                                        if (showUnconfiguredActivityDescription) {
                                            R.string.config_cd_hide_unconfigured_activity_prompt
                                        } else {
                                            R.string.config_cd_show_unconfigured_activity_prompt
                                        }
                                    ),
                                    tint = MaterialTheme.colorScheme.primary
                                )
                            }
                        }
                        if (showUnconfiguredActivityDescription) {
                            Text(
                                text = stringResource(R.string.config_description_prompt_unconfigured_activity),
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                    Switch(
                        checked = promptBeforeUnconfiguredActivityRecord,
                        onCheckedChange = onPromptBeforeUnconfiguredActivityRecordChange
                    )
                }
            }
        }
    }
}
