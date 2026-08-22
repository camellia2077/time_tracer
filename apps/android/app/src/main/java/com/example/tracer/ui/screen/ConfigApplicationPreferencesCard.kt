package com.example.tracer

import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.Row
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.unit.dp
import androidx.compose.ui.semantics.Role
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.PageTransitionStyle

@Composable
internal fun ConfigApplicationPreferencesCard(
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit,
    promptBeforeUnconfiguredActivityRecord: Boolean,
    onPromptBeforeUnconfiguredActivityRecordChange: (Boolean) -> Unit,
    pageTransitionsEnabled: Boolean,
    onPageTransitionsEnabledChange: (Boolean) -> Unit,
    pageTransitionStyle: PageTransitionStyle,
    onPageTransitionStyleChange: (PageTransitionStyle) -> Unit,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {}
) {
    var pageTransitionsExpanded by rememberSaveable { mutableStateOf(false) }

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
                        Text(
                            text = stringResource(R.string.config_label_prompt_unconfigured_activity),
                            style = MaterialTheme.typography.bodyMedium
                        )
                        Text(
                            text = stringResource(
                                if (promptBeforeUnconfiguredActivityRecord) {
                                    R.string.config_preference_enabled
                                } else {
                                    R.string.config_preference_disabled
                                }
                            ),
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    Switch(
                        checked = promptBeforeUnconfiguredActivityRecord,
                        onCheckedChange = onPromptBeforeUnconfiguredActivityRecordChange
                    )
                }
                Text(
                    text = stringResource(R.string.config_description_prompt_unconfigured_activity),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.padding(top = 4.dp)
                )
                HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp))
                ExpandableSettingsButton(
                    text = stringResource(R.string.config_title_page_transition),
                    expanded = pageTransitionsExpanded,
                    onClick = { pageTransitionsExpanded = !pageTransitionsExpanded },
                    previewContent = {
                        Text(
                            text = pageTransitionSummary(
                                enabled = pageTransitionsEnabled,
                                style = pageTransitionStyle
                            ),
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                )
                if (pageTransitionsExpanded) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(top = 12.dp),
                        verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                    ) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = stringResource(R.string.config_label_enable_page_transitions),
                                style = MaterialTheme.typography.bodyMedium
                            )
                            Text(
                                text = stringResource(R.string.config_summary_enable_page_transitions),
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        Switch(
                            checked = pageTransitionsEnabled,
                            onCheckedChange = onPageTransitionsEnabledChange
                        )
                    }
                    if (pageTransitionsEnabled) {
                        Column(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(top = 8.dp)
                                .testTag("config_page_transition_options")
                        ) {
                            PageTransitionStyle.entries.forEach { style ->
                                Row(
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .selectable(
                                            selected = pageTransitionStyle == style,
                                            onClick = { onPageTransitionStyleChange(style) },
                                            role = Role.RadioButton
                                        )
                                        .padding(vertical = 6.dp),
                                    verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                                ) {
                                    RadioButton(
                                        selected = pageTransitionStyle == style,
                                        onClick = null
                                    )
                                    Text(
                                        text = pageTransitionStyleLabel(style),
                                        style = MaterialTheme.typography.bodyMedium,
                                        modifier = Modifier.padding(start = 8.dp)
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun pageTransitionStyleLabel(style: PageTransitionStyle): String = when (style) {
    PageTransitionStyle.FADE -> stringResource(R.string.config_page_transition_fade)
    PageTransitionStyle.SLIDE -> stringResource(R.string.config_page_transition_slide)
}

@Composable
private fun pageTransitionSummary(enabled: Boolean, style: PageTransitionStyle): String =
    if (enabled) {
        pageTransitionStyleLabel(style)
    } else {
        stringResource(R.string.config_page_transition_disabled)
    }
