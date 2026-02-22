package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

private const val ABOUT_AUTHOR = "camellia2077"
private const val ABOUT_REPOSITORY = "https://github.com/camellia2077/time_tracer_cpp"
private const val ABOUT_CORE_VERSION = "0.6.2"
private const val ABOUT_ANDROID_APP_VERSION = "0.2.0"

@Composable
fun ConfigSection(
    selectedCategory: ConfigCategory,
    converterFiles: List<String>,
    reportFiles: List<String>,
    selectedFile: String,
    editableContent: String,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSelectConverter: () -> Unit,
    onSelectReports: () -> Unit,
    onRefreshFiles: () -> Unit,
    onOpenFile: (String) -> Unit,
    onImportAndroidConfigFullReplace: () -> Unit,
    onImportAndroidConfigPartialUpdate: () -> Unit,
    onExportAndroidConfig: () -> Unit,
    onEditableContentChange: (String) -> Unit,
    onSaveCurrentFile: () -> Unit,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit
) {
    val visibleFiles = when (selectedCategory) {
        ConfigCategory.CONVERTER -> converterFiles
        ConfigCategory.REPORTS -> reportFiles
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        AppearanceSettingsCard(
            themeConfig = themeConfig,
            onSetThemeColor = onSetThemeColor,
            onSetThemeMode = onSetThemeMode,
            onSetUseDynamicColor = onSetUseDynamicColor,
            onSetDarkThemeStyle = onSetDarkThemeStyle,
            appLanguage = appLanguage,
            onSetAppLanguage = onSetAppLanguage
        )

        ConfigCategorySwitchCard(
            selectedCategory = selectedCategory,
            selectedFile = selectedFile,
            visibleFiles = visibleFiles,
            onSelectConverter = onSelectConverter,
            onSelectReports = onSelectReports,
            onRefreshFiles = onRefreshFiles,
            onOpenFile = onOpenFile,
            onImportAndroidConfigFullReplace = onImportAndroidConfigFullReplace,
            onImportAndroidConfigPartialUpdate = onImportAndroidConfigPartialUpdate,
            onExportAndroidConfig = onExportAndroidConfig
        )

        if (visibleFiles.isNotEmpty()) {
            ConfigEditorCard(
                selectedFile = selectedFile,
                editableContent = editableContent,
                onEditableContentChange = onEditableContentChange,
                onSaveCurrentFile = onSaveCurrentFile
            )
        }

        ConfigAboutCard()
    }
}

@Composable
private fun ConfigAboutCard() {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_about),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            SelectionContainer {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    Text(
                        stringResource(R.string.config_about_author, ABOUT_AUTHOR),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    Text(
                        stringResource(R.string.config_about_repo, ABOUT_REPOSITORY),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    HorizontalDivider()
                    Text(
                        stringResource(R.string.config_about_core, ABOUT_CORE_VERSION),
                        style = MaterialTheme.typography.bodySmall
                    )
                    Text(
                        stringResource(R.string.config_about_app, ABOUT_ANDROID_APP_VERSION),
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        }
    }
}
