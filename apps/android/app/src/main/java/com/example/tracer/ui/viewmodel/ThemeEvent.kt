package com.example.tracer.ui.viewmodel

import com.example.tracer.data.DarkSurfaceStyle
import com.example.tracer.data.LightSurfaceStyle
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette

sealed interface ThemeEvent {
    data class SetMode(val mode: ThemeMode) : ThemeEvent
    data class SetDarkSurfaceStyle(val style: DarkSurfaceStyle) : ThemeEvent
    data class SetLightSurfaceStyle(val style: LightSurfaceStyle) : ThemeEvent
    data class SetPalette(val palette: ThemePalette) : ThemeEvent
}
