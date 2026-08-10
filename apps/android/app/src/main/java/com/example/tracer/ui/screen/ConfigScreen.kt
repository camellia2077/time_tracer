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
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalWindowInfo
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.mikepenz.aboutlibraries.Libs
import com.mikepenz.aboutlibraries.ui.compose.m3.LibrariesContainer

private const val ABOUT_AUTHOR = "camellia2077"
private const val ABOUT_REPOSITORY = "https://github.com/camellia2077/time_tracer_cpp"
private const val ABOUT_LOG_TAG = "ConfigAboutPage"

private data class LibrariesLoadState(
    val libraries: Libs? = null,
    val isLoading: Boolean = true,
    val hasError: Boolean = false
)

@Composable
internal fun ConfigSection(
    selectedCategory: ConfigCategory,
    aliasFiles: List<ConfigTomlFileEntry>,
    chartFiles: List<ConfigTomlFileEntry>,
    metaFiles: List<ConfigTomlFileEntry>,
    insightsFiles: List<ConfigTomlFileEntry>,
    selectedFilePath: String,
    selectedFileDisplayName: String,
    selectedFileContent: String,
    editableContent: String,
    aliasEditorMode: AliasEditorMode,
    aliasDocumentDraft: ActivityHierarchyDocument?,
    aliasEntryMovePlan: AliasEntryMovePlan?,
    aliasEntryMoveDestinations: List<AliasEntryMoveDestinationDocument>,
    aliasEntryMoveDestinationsLoading: Boolean,
    aliasAdvancedTomlDraft: String,
    aliasEditorErrorMessage: String,
    autoSaveStatus: ConfigAutoSaveStatus,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSelectAlias: () -> Unit,
    onSelectCharts: () -> Unit,
    onSelectMeta: () -> Unit,
    onSelectInsights: () -> Unit,
    onRefreshFiles: () -> Unit,
    onOpenFile: (String) -> Unit,
    onCreateAliasTomlFile: (String) -> Unit,
    onDeleteAliasTomlFile: () -> Unit,
    onRenameAliasCategory: (String) -> Unit,
    onCopyDiagnosticsPayload: () -> Unit,
    onEditableContentChange: (String) -> Unit,
    onSelectAliasStructuredMode: () -> Unit,
    onSelectAliasAdvancedMode: () -> Unit,
    onAliasAdvancedTomlChange: (String) -> Unit,
    onAddAliasGroup: (String?, String) -> Unit,
    onDeleteAliasGroup: (String) -> Unit,
    onRenameAliasGroup: (String, String) -> Unit,
    onAddAliasEntry: (String?, String, List<String>) -> Unit,
    onUpdateAliasEntry: (String, String, List<String>) -> Unit,
    onMergeAliasEntry: (String, String) -> Unit,
    onPromoteAliasEntry: (String) -> Unit,
    onRenameGroupAlias: (String, String, String) -> Unit,
    onAddGroupAlias: (String, String) -> Unit,
    onUpdateGroupAliases: (String, List<String>) -> Unit,
    onDeleteAliasEntry: (String) -> Unit,
    onPrepareAliasEntryMove: (String) -> Unit,
    onPrepareAliasGroupMove: (String) -> Unit,
    onPreviewAliasEntryMove: (String, AliasEntryMoveTarget) -> Unit,
    onPreviewAliasGroupMove: (String, AliasEntryMoveTarget) -> Unit,
    onConfirmAliasEntryMovePlan: () -> Unit,
    onDiscardAliasEntryMovePlan: () -> Unit,
    onSaveCurrentFile: () -> Unit,
    onThemeEvent: (com.example.tracer.ui.viewmodel.ThemeEvent) -> Unit,
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    insightsAverageDayBasis: InsightsAverageDayBasis,
    onInsightsAverageDayBasisChange: (InsightsAverageDayBasis) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    extraContent: @Composable () -> Unit = {}
) {
    var showAboutPage by rememberSaveable { mutableStateOf(false) }
    val visibleFiles = when (selectedCategory) {
        ConfigCategory.ALIAS -> aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }
        ConfigCategory.CHARTS -> chartFiles
        ConfigCategory.META -> metaFiles
        ConfigCategory.INSIGHTS -> insightsFiles
    }.map { entry ->
        entry.copy(displayName = displayNameForCurrentScope(entry, selectedCategory))
    }
    val scopedSelectedFileDisplayName = selectedFileDisplayName.removeCurrentScopePrefix(
        selectedCategory = selectedCategory
    )
    val usesAliasStructuredEditor = selectedFilePath.isAliasFilePathForConfigScreen()

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
        ConfigApplicationPreferencesCard(
            appLanguage = appLanguage,
            onSetAppLanguage = onSetAppLanguage
        )
        AppearanceSettingsCard(
            themeConfig = themeConfig,
            onThemeEvent = onThemeEvent
        )
        ConfigInsightsAverageDayBasisCard(
            insightsPiePalettePreset = insightsPiePalettePreset,
            onInsightsPiePalettePresetChange = onInsightsPiePalettePresetChange,
            selected = insightsAverageDayBasis,
            onSelected = onInsightsAverageDayBasisChange
        )

        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.fillMaxWidth()) {
                ConfigCategorySwitchCard(
                    selectedCategory = selectedCategory,
                    onSelectAlias = onSelectAlias,
                    onSelectCharts = onSelectCharts,
                    onSelectMeta = onSelectMeta,
                    onSelectInsights = onSelectInsights,
                    onRefreshFiles = onRefreshFiles
                )

                if (visibleFiles.isNotEmpty() || selectedCategory == ConfigCategory.ALIAS) {
                    HorizontalDivider()
                }

                if (visibleFiles.isNotEmpty()) {
                    if (usesAliasStructuredEditor) {
                        ConfigAliasEditorCard(
                    aliasFiles = visibleFiles,
                    selectedFileDisplayName = scopedSelectedFileDisplayName,
                    selectedFileContent = selectedFileContent,
                    mode = aliasEditorMode,
                    document = aliasDocumentDraft,
                    movePlan = aliasEntryMovePlan,
                    moveDestinations = aliasEntryMoveDestinations,
                    moveDestinationsLoading = aliasEntryMoveDestinationsLoading,
                    advancedTomlDraft = aliasAdvancedTomlDraft,
                    errorMessage = aliasEditorErrorMessage,
                    onCreateAliasTomlFile = onCreateAliasTomlFile,
                    onSelectAliasFile = onOpenFile,
                    onDeleteAliasTomlFile = onDeleteAliasTomlFile,
                    onRenameCategory = onRenameAliasCategory,
                    onSelectStructuredMode = onSelectAliasStructuredMode,
                    onSelectAdvancedMode = onSelectAliasAdvancedMode,
                    onAdvancedTomlChange = onAliasAdvancedTomlChange,
                    onAddGroup = onAddAliasGroup,
                    onDeleteGroup = onDeleteAliasGroup,
                    onRenameGroup = onRenameAliasGroup,
                    onAddEntry = onAddAliasEntry,
                    onUpdateEntry = onUpdateAliasEntry,
                    onMergeEntry = onMergeAliasEntry,
                    onPromoteEntry = onPromoteAliasEntry,
                    onRenameGroupAlias = onRenameGroupAlias,
                    onAddGroupAlias = onAddGroupAlias,
                    onUpdateGroupAliases = onUpdateGroupAliases,
                    onDeleteEntry = onDeleteAliasEntry,
                    onPrepareEntryMove = onPrepareAliasEntryMove,
                    onPrepareGroupMove = onPrepareAliasGroupMove,
                    onPreviewEntryMove = onPreviewAliasEntryMove,
                    onPreviewGroupMove = onPreviewAliasGroupMove,
                    onConfirmMovePlan = onConfirmAliasEntryMovePlan,
                    onDiscardMovePlan = onDiscardAliasEntryMovePlan,
                    onSave = onSaveCurrentFile
                        )
                    } else {
                        ConfigEditorCard(
                    selectedFileDisplayName = scopedSelectedFileDisplayName,
                    selectedFileContent = selectedFileContent,
                    editableContent = editableContent,
                    autoSaveStatus = autoSaveStatus,
                    onEditableContentChange = onEditableContentChange,
                    onSaveCurrentFile = onSaveCurrentFile,
                    readOnly = selectedCategory != ConfigCategory.ALIAS
                        )
                    }
                } else if (selectedCategory == ConfigCategory.ALIAS) {
                    ConfigAliasEmptyFileCard(
                        onCreateAliasTomlFile = onCreateAliasTomlFile
                    )
                }
            }
        }

        extraContent()

        ConfigAboutCard(
            onOpenAbout = { showAboutPage = true },
            onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
        )
    }
}

@Composable
private fun ConfigAliasEmptyFileCard(
    onCreateAliasTomlFile: (String) -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
            Text(
                text = stringResource(R.string.config_alias_empty_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.config_alias_empty_message),
                style = MaterialTheme.typography.bodyMedium
            )
            ConfigEditorFileControls(
                onCreateAliasTomlFile = onCreateAliasTomlFile
            )
    }
}

private fun displayNameForCurrentScope(
    entry: ConfigTomlFileEntry,
    selectedCategory: ConfigCategory
): String {
    // Keep canonical path and stable displayName untouched in state/runtime.
    // Only trim the in-scope subcategory prefix at render time when the user
    // has already opened the Alias category.
    return entry.displayName.removeCurrentScopePrefix(
        selectedCategory = selectedCategory
    )
}

private fun String.removeCurrentScopePrefix(
    selectedCategory: ConfigCategory
): String {
    return if (selectedCategory == ConfigCategory.ALIAS) {
        removePrefix("user/activity_hierarchy/")
    } else {
        this
    }
}

private fun String.isAliasFilePathForConfigScreen(): Boolean =
        startsWith("user/activity_hierarchy/") &&
        endsWith(".toml", ignoreCase = true)

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
    val licensePanelMaxHeight = LocalWindowInfo.current.containerDpSize.height * 0.6f
    val librariesLoadState by produceState(initialValue = LibrariesLoadState(), context) {
        value = runCatching {
            val libs = AboutLibrariesAssetLoader.load(context)
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
                        stringResource(R.string.config_about_core, BuildConfig.TRACER_CORE_VERSION),
                        style = MaterialTheme.typography.bodySmall
                    )
                    Text(
                        stringResource(R.string.config_about_app, BuildConfig.VERSION_NAME),
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
