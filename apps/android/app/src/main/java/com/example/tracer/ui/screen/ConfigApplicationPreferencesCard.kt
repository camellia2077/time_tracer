package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.data.AppLanguage

@Composable
internal fun ConfigApplicationPreferencesCard(
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {}
) {
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
            }
        }
    }
}
