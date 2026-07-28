package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.data.AppLanguage

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun LanguageSection(
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit
) {
    var showLanguageSheet by rememberSaveable { mutableStateOf(false) }
    val appLanguages = listOf(
        AppLanguage.Chinese to stringResource(R.string.config_language_chinese),
        AppLanguage.English to stringResource(R.string.config_language_english),
        AppLanguage.Japanese to stringResource(R.string.config_language_japanese)
    )
    val selectedLabel = appLanguages.first { it.first == appLanguage }.second

    Text(
        text = stringResource(R.string.config_title_language),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    ExpandableSettingsButton(
        text = selectedLabel,
        expanded = showLanguageSheet,
        onClick = { showLanguageSheet = true }
    )

    if (showLanguageSheet) {
        ModalBottomSheet(
            onDismissRequest = { showLanguageSheet = false },
            sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(2.dp)
            ) {
                Text(
                    text = stringResource(R.string.config_title_language),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.padding(bottom = 8.dp)
                )
                appLanguages.forEach { (language, label) ->
                    TextButton(
                        onClick = {
                            showLanguageSheet = false
                            onSetAppLanguage(language)
                        },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            text = label,
                            color = if (language == appLanguage) {
                                MaterialTheme.colorScheme.primary
                            } else {
                                MaterialTheme.colorScheme.onSurface
                            }
                        )
                    }
                }
            }
        }
    }
}
