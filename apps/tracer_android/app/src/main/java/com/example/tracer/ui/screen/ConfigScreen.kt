package com.example.tracer

import android.util.Log
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.produceState
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.mikepenz.aboutlibraries.Libs
import com.mikepenz.aboutlibraries.util.withContext
import com.mikepenz.aboutlibraries.ui.compose.m3.LibrariesContainer

private const val ABOUT_AUTHOR = "camellia2077"
private const val ABOUT_REPOSITORY = "https://github.com/camellia2077/time_tracer_cpp"
private const val ABOUT_CORE_VERSION = "0.6.2"
private const val ABOUT_ANDROID_APP_VERSION = "0.2.2"
private const val ABOUT_LOG_TAG = "ConfigAboutPage"

private data class LibrariesLoadState(
    val libraries: Libs? = null,
    val isLoading: Boolean = true,
    val hasError: Boolean = false
)

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
    onCopyDiagnosticsPayload: () -> Unit,
    onEditableContentChange: (String) -> Unit,
    onSaveCurrentFile: () -> Unit,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit
) {
    var showAboutPage by rememberSaveable { mutableStateOf(false) }
    val visibleFiles = when (selectedCategory) {
        ConfigCategory.CONVERTER -> converterFiles
        ConfigCategory.REPORTS -> reportFiles
    }

    if (showAboutPage) {
        ConfigAboutPage(
            onBack = { showAboutPage = false },
            onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
        )
        return
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

        ConfigAboutCard(
            onOpenAbout = { showAboutPage = true },
            onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
        )
    }
}

@Composable
private fun ConfigAboutCard(
    onOpenAbout: () -> Unit,
    onCopyDiagnosticsPayload: () -> Unit
) {
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

            Text(
                text = stringResource(R.string.config_about_entry_description),
                style = MaterialTheme.typography.bodyMedium
            )

            OutlinedButton(
                onClick = onOpenAbout,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_action_open_about))
            }

            OutlinedButton(
                onClick = onCopyDiagnosticsPayload,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_action_copy_diagnostics))
            }
        }
    }
}

@Composable
private fun ConfigAboutPage(
    onBack: () -> Unit,
    onCopyDiagnosticsPayload: () -> Unit
) {
    val context = LocalContext.current
    val licensePanelMaxHeight = LocalConfiguration.current.screenHeightDp.dp * 0.6f
    val librariesLoadState by produceState(initialValue = LibrariesLoadState(), context) {
        value = runCatching {
            val libs = Libs.Builder()
                .withContext(context)
                .build()
            LibrariesLoadState(libraries = libs, isLoading = false)
        }.getOrElse { error ->
            Log.e(ABOUT_LOG_TAG, "Failed to load open-source licenses metadata.", error)
            LibrariesLoadState(isLoading = false, hasError = true)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 8.dp, vertical = 4.dp)
            ) {
                IconButton(onClick = onBack) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = stringResource(R.string.config_action_back)
                    )
                }
                Text(
                    text = stringResource(R.string.config_title_about),
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier
                        .align(Alignment.Center)
                )
            }
        }

        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Text(
                text = stringResource(R.string.config_title_project_info),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 16.dp)
            )
            SelectionContainer {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
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
            OutlinedButton(
                onClick = onCopyDiagnosticsPayload,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(stringResource(R.string.config_action_copy_diagnostics))
            }
        }

        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Text(
                text = stringResource(R.string.config_title_open_source_licenses),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 12.dp)
            )
            Text(
                text = stringResource(R.string.config_open_source_licenses_description),
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 4.dp)
            )
            HorizontalDivider(modifier = Modifier.padding(top = 8.dp))

            when {
                librariesLoadState.isLoading -> {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .heightIn(min = 220.dp, max = licensePanelMaxHeight),
                        contentAlignment = Alignment.Center
                    ) {
                        CircularProgressIndicator()
                    }
                }
                librariesLoadState.hasError || librariesLoadState.libraries == null -> {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .heightIn(min = 160.dp, max = licensePanelMaxHeight)
                            .padding(16.dp),
                        contentAlignment = Alignment.TopStart
                    ) {
                        Text(
                            text = stringResource(R.string.config_open_source_licenses_unavailable),
                            style = MaterialTheme.typography.bodyMedium
                        )
                    }
                }
                else -> {
                    LibrariesContainer(
                        libraries = librariesLoadState.libraries,
                        modifier = Modifier
                            .fillMaxWidth()
                            .heightIn(min = 240.dp, max = licensePanelMaxHeight)
                    )
                }
            }
        }
    }
}
