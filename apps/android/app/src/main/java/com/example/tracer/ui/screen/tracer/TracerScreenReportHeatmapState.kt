package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

internal data class TracerScreenReportHeatmapState(
    val config: ReportHeatmapTomlConfig,
    val stylePreference: ReportHeatmapStylePreference,
    val applyMessage: String,
    val onThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    val onPaletteNameChange: (String) -> Unit
)

@Composable
internal fun rememberTracerScreenReportHeatmapState(
    selectedTab: TracerTab,
    configGateway: ConfigGateway
): TracerScreenReportHeatmapState {
    val coroutineScope = rememberCoroutineScope()
    var reportHeatmapTomlConfig by remember { mutableStateOf(defaultReportHeatmapTomlConfig()) }
    var reportHeatmapStylePreference by remember {
        mutableStateOf(
            ReportHeatmapTomlLoader.deriveStylePreference(reportHeatmapTomlConfig)
        )
    }
    var reportHeatmapApplyMessage by remember { mutableStateOf("") }
    var reportHeatmapAutoApplyJob by remember { mutableStateOf<Job?>(null) }

    LaunchedEffect(selectedTab, configGateway) {
        if (selectedTab == TracerTab.REPORT) {
            val loadedConfig = withContext(Dispatchers.IO) {
                ReportHeatmapTomlLoader.load(configGateway)
            }
            reportHeatmapTomlConfig = loadedConfig
            reportHeatmapStylePreference =
                ReportHeatmapTomlLoader.deriveStylePreference(loadedConfig)
            reportHeatmapApplyMessage = ""
        }
    }

    fun scheduleHeatmapTomlAutoApply(nextStyle: ReportHeatmapStylePreference) {
        val targetConfig = reportHeatmapTomlConfig
        reportHeatmapAutoApplyJob?.cancel()
        reportHeatmapApplyMessage = ""
        reportHeatmapAutoApplyJob = coroutineScope.launch {
            val applyResult = withContext(Dispatchers.IO) {
                applyHeatmapStyleToToml(
                    configGateway = configGateway,
                    config = targetConfig,
                    style = nextStyle
                )
            }
            if (!applyResult.ok) {
                reportHeatmapApplyMessage = applyResult.message
                return@launch
            }
            reportHeatmapApplyMessage = ""
            if (applyResult.updatedConfig != null) {
                reportHeatmapTomlConfig = applyResult.updatedConfig
            }
            if (applyResult.updatedStyle != null) {
                reportHeatmapStylePreference = applyResult.updatedStyle
            }
        }
    }

    return TracerScreenReportHeatmapState(
        config = reportHeatmapTomlConfig,
        stylePreference = reportHeatmapStylePreference,
        applyMessage = reportHeatmapApplyMessage,
        onThemePolicyChange = { value ->
            val nextStyle = when (value) {
                ReportHeatmapThemePolicy.FOLLOW_SYSTEM -> {
                    ReportHeatmapStylePreference(
                        themePolicy = ReportHeatmapThemePolicy.FOLLOW_SYSTEM,
                        paletteName = ""
                    )
                }

                ReportHeatmapThemePolicy.PALETTE -> {
                    reportHeatmapStylePreference.copy(
                        themePolicy = ReportHeatmapThemePolicy.PALETTE
                    )
                }
            }
            if (nextStyle != reportHeatmapStylePreference) {
                reportHeatmapStylePreference = nextStyle
                scheduleHeatmapTomlAutoApply(nextStyle)
            }
        },
        onPaletteNameChange = { value ->
            val nextStyle = reportHeatmapStylePreference.copy(
                paletteName = value.trim()
            )
            if (nextStyle != reportHeatmapStylePreference) {
                reportHeatmapStylePreference = nextStyle
                scheduleHeatmapTomlAutoApply(nextStyle)
            }
        }
    )
}

private data class HeatmapTomlApplyResult(
    val ok: Boolean,
    val message: String,
    val updatedConfig: ReportHeatmapTomlConfig? = null,
    val updatedStyle: ReportHeatmapStylePreference? = null
)

private suspend fun applyHeatmapStyleToToml(
    configGateway: ConfigGateway,
    config: ReportHeatmapTomlConfig,
    style: ReportHeatmapStylePreference
): HeatmapTomlApplyResult {
    val configPath = ReportHeatmapTomlLoader.configPath()
    val readResult = configGateway.readConfigTomlFile(configPath)
    if (!readResult.ok) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = readResult.message.ifBlank {
                "Apply failed: cannot read $configPath."
            }
        )
    }

    val (nextLightPalette, nextDarkPalette) = ReportHeatmapTomlLoader
        .resolveDefaultPalettesForStyle(
            config = config,
            stylePreference = style
        )
    val rewrittenContent = ReportHeatmapTomlLoader.rewriteDefaults(
        rawToml = readResult.content,
        lightPalette = nextLightPalette,
        darkPalette = nextDarkPalette
    )
    if (rewrittenContent == null) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = "Apply failed: missing [defaults] section in $configPath."
        )
    }

    val saveResult = configGateway.saveConfigTomlFile(configPath, rewrittenContent)
    if (!saveResult.ok) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = saveResult.message.ifBlank {
                "Apply failed: cannot save $configPath."
            }
        )
    }

    val updatedConfig = ReportHeatmapTomlLoader.parse(rewrittenContent)
    return HeatmapTomlApplyResult(
        ok = true,
        message = saveResult.message.ifBlank {
            "Applied heatmap style to $configPath."
        },
        updatedConfig = updatedConfig,
        updatedStyle = ReportHeatmapTomlLoader.deriveStylePreference(updatedConfig)
    )
}
