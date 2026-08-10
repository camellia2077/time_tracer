package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import kotlinx.coroutines.launch
import com.example.tracer.data.UserPreferencesRepository

internal data class TracerScreenInsightsHeatmapState(
    val config: InsightsHeatmapTomlConfig,
    val stylePreference: InsightsHeatmapStylePreference,
    val applyMessage: String,
    val onThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    val onPaletteNameChange: (String) -> Unit
)

@Composable
internal fun rememberTracerScreenInsightsHeatmapState(
    selectedTab: TracerTab,
    configGateway: ConfigGateway,
    userPreferencesRepository: UserPreferencesRepository
): TracerScreenInsightsHeatmapState {
    val coroutineScope = rememberCoroutineScope()
    val savedPaletteName by userPreferencesRepository.insightsHeatmapPaletteName.collectAsState(
        initial = ""
    )
    var insightsHeatmapTomlConfig by remember { mutableStateOf(defaultInsightsHeatmapTomlConfig()) }
    var insightsHeatmapStylePreference by remember {
        mutableStateOf(
            InsightsHeatmapTomlLoader.deriveStylePreference(insightsHeatmapTomlConfig)
        )
    }
    var insightsHeatmapApplyMessage by remember { mutableStateOf("") }

    LaunchedEffect(selectedTab, configGateway, savedPaletteName) {
        if (selectedTab == TracerTab.INSIGHTS) {
            val loadedConfig = InsightsHeatmapTomlLoader.load(configGateway)
            insightsHeatmapTomlConfig = loadedConfig
            val selectedPalette = savedPaletteName
                .takeIf { it in loadedConfig.palettes }
                ?: loadedConfig.paletteNames().firstOrNull().orEmpty()
            insightsHeatmapStylePreference = InsightsHeatmapStylePreference(
                themePolicy = InsightsHeatmapThemePolicy.PALETTE,
                paletteName = selectedPalette,
            )
            insightsHeatmapApplyMessage = ""
        }
    }

    fun persistHeatmapPalette(nextStyle: InsightsHeatmapStylePreference) {
        insightsHeatmapApplyMessage = ""
        if (nextStyle.themePolicy != InsightsHeatmapThemePolicy.PALETTE ||
            nextStyle.paletteName.isBlank()) {
            return
        }
        coroutineScope.launch {
            userPreferencesRepository.setInsightsHeatmapPaletteName(nextStyle.paletteName)
        }
    }

    return TracerScreenInsightsHeatmapState(
        config = insightsHeatmapTomlConfig,
        stylePreference = insightsHeatmapStylePreference,
        applyMessage = insightsHeatmapApplyMessage,
        onThemePolicyChange = { value ->
            val nextStyle = when (value) {
                InsightsHeatmapThemePolicy.FOLLOW_SYSTEM -> {
                    InsightsHeatmapStylePreference(
                        themePolicy = InsightsHeatmapThemePolicy.FOLLOW_SYSTEM,
                        paletteName = ""
                    )
                }

                InsightsHeatmapThemePolicy.PALETTE -> {
                    insightsHeatmapStylePreference.copy(
                        themePolicy = InsightsHeatmapThemePolicy.PALETTE
                    )
                }
            }
            if (nextStyle != insightsHeatmapStylePreference) {
                insightsHeatmapStylePreference = nextStyle
                persistHeatmapPalette(nextStyle)
            }
        },
        onPaletteNameChange = { value ->
            val nextStyle = insightsHeatmapStylePreference.copy(
                themePolicy = InsightsHeatmapThemePolicy.PALETTE,
                paletteName = value.trim()
            )
            if (nextStyle != insightsHeatmapStylePreference) {
                insightsHeatmapStylePreference = nextStyle
                persistHeatmapPalette(nextStyle)
            }
        }
    )
}
