package com.example.tracer

import android.util.Log
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
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
import com.example.tracer.data.ConfigCard
import com.example.tracer.data.ConfigCardExpansionPreferences
import com.mikepenz.aboutlibraries.Libs
import com.mikepenz.aboutlibraries.ui.compose.m3.LibrariesContainer

private const val ABOUT_AUTHOR = "camellia2077"
private const val ABOUT_REPOSITORY = "https://github.com/camellia2077/time_tracer"
private const val ABOUT_LOG_TAG = "ConfigAboutPage"

private data class LibrariesLoadState(
    val libraries: Libs? = null,
    val isLoading: Boolean = true,
    val hasError: Boolean = false
)

private fun Libs.filterForQuery(query: String): Libs {
    val normalizedQuery = query.trim()
    if (normalizedQuery.isEmpty()) return this

    return copy(
        libraries = libraries.filter { library ->
            library.name.orEmpty().contains(normalizedQuery, ignoreCase = true) ||
                library.artifactId.orEmpty().contains(normalizedQuery, ignoreCase = true) ||
                library.description.orEmpty().contains(normalizedQuery, ignoreCase = true) ||
                library.developers.any { developer ->
                    developer.name.orEmpty().contains(normalizedQuery, ignoreCase = true)
                } ||
                library.licenses.any { license ->
                    license.name.orEmpty().contains(normalizedQuery, ignoreCase = true)
                }
        }
    )
}

private enum class ConfigAboutDestination {
    PROJECT_DETAILS,
    THIRD_PARTY_LICENSES
}

@Composable
internal fun ConfigSection(
    themeConfig: com.example.tracer.data.ThemeConfig,
    onCopyDiagnosticsPayload: () -> Unit,
    onThemeEvent: (com.example.tracer.ui.viewmodel.ThemeEvent) -> Unit,
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    insightsComparisonColorScheme: InsightsComparisonColorScheme,
    onInsightsComparisonColorSchemeChange: (InsightsComparisonColorScheme) -> Unit,
    insightsComparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    onInsightsComparisonIndicatorStyleChange: (InsightsComparisonIndicatorStyle) -> Unit,
    insightsAverageDayBasis: InsightsAverageDayBasis,
    onInsightsAverageDayBasisChange: (InsightsAverageDayBasis) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    promptBeforeUnconfiguredActivityRecord: Boolean,
    onPromptBeforeUnconfiguredActivityRecordChange: (Boolean) -> Unit,
    pageTransitionsEnabled: Boolean,
    onPageTransitionsEnabledChange: (Boolean) -> Unit,
    pageTransitionStyle: com.example.tracer.data.PageTransitionStyle,
    onPageTransitionStyleChange: (com.example.tracer.data.PageTransitionStyle) -> Unit,
    cardExpansionPreferences: ConfigCardExpansionPreferences,
    onConfigCardExpandedChange: (ConfigCard, Boolean) -> Unit,
    extraContent: @Composable () -> Unit = {}
) {
    var aboutDestination by rememberSaveable { mutableStateOf<ConfigAboutDestination?>(null) }

    when (aboutDestination) {
        ConfigAboutDestination.PROJECT_DETAILS -> {
            ConfigProjectDetailsPage(
                onBack = { aboutDestination = null },
                onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
            )
            return
        }
        ConfigAboutDestination.THIRD_PARTY_LICENSES -> {
            ConfigThirdPartyLicensesPage(onBack = { aboutDestination = null })
            return
        }
        null -> Unit
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        ConfigApplicationPreferencesCard(
            appLanguage = appLanguage,
            onSetAppLanguage = onSetAppLanguage,
            promptBeforeUnconfiguredActivityRecord = promptBeforeUnconfiguredActivityRecord,
            onPromptBeforeUnconfiguredActivityRecordChange =
                onPromptBeforeUnconfiguredActivityRecordChange,
            pageTransitionsEnabled = pageTransitionsEnabled,
            onPageTransitionsEnabledChange = onPageTransitionsEnabledChange,
            pageTransitionStyle = pageTransitionStyle,
            onPageTransitionStyleChange = onPageTransitionStyleChange,
            expanded = cardExpansionPreferences.applicationPreferencesExpanded,
            onToggleExpanded = {
                onConfigCardExpandedChange(
                    ConfigCard.APPLICATION_PREFERENCES,
                    !cardExpansionPreferences.applicationPreferencesExpanded
                )
            }
        )
        AppearanceSettingsCard(
            themeConfig = themeConfig,
            onThemeEvent = onThemeEvent,
            themePaletteExpanded = cardExpansionPreferences.themePaletteExpanded,
            onThemePaletteExpandedChange = { value ->
                onConfigCardExpandedChange(ConfigCard.THEME_PALETTE, value)
            },
            expanded = cardExpansionPreferences.appearanceExpanded,
            onToggleExpanded = {
                onConfigCardExpandedChange(ConfigCard.APPEARANCE, !cardExpansionPreferences.appearanceExpanded)
            }
        )
        ConfigInsightsAverageDayBasisCard(
            insightsPiePalettePreset = insightsPiePalettePreset,
            onInsightsPiePalettePresetChange = onInsightsPiePalettePresetChange,
            comparisonColorScheme = insightsComparisonColorScheme,
            onComparisonColorSchemeChange = onInsightsComparisonColorSchemeChange,
            comparisonIndicatorStyle = insightsComparisonIndicatorStyle,
            onComparisonIndicatorStyleChange = onInsightsComparisonIndicatorStyleChange,
            insightsChartStyleExpanded = cardExpansionPreferences.insightsChartStyleExpanded,
            onInsightsChartStyleExpandedChange = { value ->
                onConfigCardExpandedChange(ConfigCard.INSIGHTS_CHART_STYLE, value)
            },
            insightsComparisonExpanded = cardExpansionPreferences.insightsComparisonExpanded,
            onInsightsComparisonExpandedChange = { value ->
                onConfigCardExpandedChange(ConfigCard.INSIGHTS_COMPARISON, value)
            },
            selected = insightsAverageDayBasis,
            onSelected = onInsightsAverageDayBasisChange,
            expanded = cardExpansionPreferences.insightsSettingsExpanded,
            onToggleExpanded = {
                onConfigCardExpandedChange(
                    ConfigCard.INSIGHTS_SETTINGS,
                    !cardExpansionPreferences.insightsSettingsExpanded
                )
            }
        )

        extraContent()

        ConfigAboutCard(
            onOpenProjectDetails = {
                aboutDestination = ConfigAboutDestination.PROJECT_DETAILS
            },
            onOpenThirdPartyLicenses = {
                aboutDestination = ConfigAboutDestination.THIRD_PARTY_LICENSES
            },
            expanded = cardExpansionPreferences.aboutExpanded,
            onToggleExpanded = {
                onConfigCardExpandedChange(ConfigCard.ABOUT, !cardExpansionPreferences.aboutExpanded)
            }
        )
    }
}

@Composable
private fun ConfigAboutCard(
    onOpenProjectDetails: () -> Unit,
    onOpenThirdPartyLicenses: () -> Unit,
    expanded: Boolean,
    onToggleExpanded: () -> Unit
) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            ConfigCardHeader(
                title = stringResource(R.string.config_title_about),
                expanded = expanded,
                onToggleExpanded = onToggleExpanded
            )
            if (expanded) {
                ConfigAboutNavigationItem(
                    title = stringResource(R.string.config_title_project_info),
                    summary = stringResource(R.string.config_project_details_description),
                    onClick = onOpenProjectDetails
                )
                ConfigAboutNavigationItem(
                    title = stringResource(R.string.config_title_open_source_licenses),
                    summary = stringResource(R.string.config_open_source_licenses_description),
                    onClick = onOpenThirdPartyLicenses
                )
            }
        }
    }
}

@Composable
private fun ConfigAboutNavigationItem(
    title: String,
    summary: String,
    onClick: () -> Unit
) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        shape = MaterialTheme.shapes.medium,
        color = MaterialTheme.colorScheme.surface,
        contentColor = MaterialTheme.colorScheme.onSurface,
        border = BorderStroke(1.dp, MaterialTheme.colorScheme.outlineVariant)
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(text = title, style = MaterialTheme.typography.bodyMedium)
                Text(
                    text = summary,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Spacer(modifier = Modifier.width(8.dp))
            Icon(
                imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                contentDescription = null
            )
        }
    }
}

@Composable
private fun ConfigProjectDetailsPage(
    onBack: () -> Unit,
    onCopyDiagnosticsPayload: () -> Unit
) {
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
                    text = stringResource(R.string.config_title_project_info),
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

    }
}

@Composable
private fun ConfigThirdPartyLicensesPage(onBack: () -> Unit) {
    val context = LocalContext.current
    // This screen is hosted in Config's scroll container, so give the inner lazy list a
    // viewport that reaches the floating navigation area instead of capping it at 60%.
    val licensePanelMaxHeight = LocalWindowInfo.current.containerDpSize.height * 0.72f
    var searchQuery by rememberSaveable { mutableStateOf("") }
    val librariesLoadState by produceState(initialValue = LibrariesLoadState(), context) {
        value = runCatching {
            val libs = AboutLibrariesAssetLoader.load(context)
            LibrariesLoadState(libraries = libs, isLoading = false)
        }.getOrElse { error ->
            Log.e(ABOUT_LOG_TAG, "Failed to load third-party licenses metadata.", error)
            LibrariesLoadState(isLoading = false, hasError = true)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
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
                    text = stringResource(R.string.config_title_open_source_licenses),
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.align(Alignment.Center)
                )
            }
        }

        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Text(
                text = stringResource(R.string.config_open_source_licenses_description),
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 12.dp)
            )
            OutlinedTextField(
                value = searchQuery,
                onValueChange = { searchQuery = it },
                label = { Text(stringResource(R.string.config_open_source_licenses_search)) },
                singleLine = true,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(start = 16.dp, end = 16.dp, bottom = 12.dp)
            )
            HorizontalDivider()

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
                    val loadedLibraries = requireNotNull(librariesLoadState.libraries)
                    val filteredLibraries = loadedLibraries.filterForQuery(searchQuery)
                    if (filteredLibraries.libraries.isEmpty()) {
                        Box(
                            modifier = Modifier
                                .fillMaxWidth()
                                .heightIn(min = 240.dp, max = licensePanelMaxHeight)
                                .padding(16.dp),
                            contentAlignment = Alignment.TopStart
                        ) {
                            Text(
                                text = stringResource(R.string.config_open_source_licenses_no_results),
                                style = MaterialTheme.typography.bodyMedium
                            )
                        }
                    } else {
                        LibrariesContainer(
                            libraries = filteredLibraries,
                            modifier = Modifier
                                .fillMaxWidth()
                                .heightIn(min = 240.dp, max = licensePanelMaxHeight)
                        )
                    }
                }
            }
        }
    }
}
