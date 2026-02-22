package com.example.tracer

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier

@Composable
internal fun Modifier.tracerTabContentModifier(
    selectedTab: TracerTab,
    innerPadding: PaddingValues
): Modifier {
    val baseModifier = this
        .fillMaxSize()
        .padding(innerPadding)
        .padding(
            start = ScreenOuterPadding,
            end = ScreenOuterPadding,
            bottom = ScreenOuterPadding
        )
    val selectedEntry = TracerTabRegistry.entry(selectedTab)
    return if (selectedEntry.scrollBehavior == TracerTabScrollBehavior.VERTICAL) {
        baseModifier.verticalScroll(rememberScrollState())
    } else {
        baseModifier
    }
}

@Composable
internal fun TracerTabRouteContent(
    modifier: Modifier = Modifier,
    selectedTab: TracerTab,
    dataViewModel: DataViewModel,
    queryUiState: QueryReportUiState,
    queryReportViewModel: QueryReportViewModel,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    configUiState: ConfigUiState,
    configViewModel: ConfigViewModel,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    validMappingNames: Set<String>,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    onImportSingleTxt: () -> Unit,
    onExportAllMonthsTxt: () -> Unit,
    isTxtExportInProgress: Boolean,
    onImportAndroidConfigFullReplace: () -> Unit,
    onImportAndroidConfigPartialUpdate: () -> Unit,
    onExportAndroidConfigBundle: () -> Unit
) {
    val args = TracerTabRouteArgs(
        dataViewModel = dataViewModel,
        queryUiState = queryUiState,
        queryReportViewModel = queryReportViewModel,
        recordUiState = recordUiState,
        recordViewModel = recordViewModel,
        configUiState = configUiState,
        configViewModel = configViewModel,
        themeConfig = themeConfig,
        onSetThemeColor = onSetThemeColor,
        onSetThemeMode = onSetThemeMode,
        onSetUseDynamicColor = onSetUseDynamicColor,
        onSetDarkThemeStyle = onSetDarkThemeStyle,
        appLanguage = appLanguage,
        onSetAppLanguage = onSetAppLanguage,
        validMappingNames = validMappingNames,
        userPreferencesRepository = userPreferencesRepository,
        coroutineScope = coroutineScope,
        onImportSingleTxt = onImportSingleTxt,
        onExportAllMonthsTxt = onExportAllMonthsTxt,
        isTxtExportInProgress = isTxtExportInProgress,
        onImportAndroidConfigFullReplace = onImportAndroidConfigFullReplace,
        onImportAndroidConfigPartialUpdate = onImportAndroidConfigPartialUpdate,
        onExportAndroidConfigBundle = onExportAndroidConfigBundle
    )
    TracerTabRegistry.entry(selectedTab).content(modifier, args)
}
