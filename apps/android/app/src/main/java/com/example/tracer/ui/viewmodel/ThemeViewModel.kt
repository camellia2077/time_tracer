package com.example.tracer.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.UserPreferencesRepository
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

class ThemeViewModel(private val repository: UserPreferencesRepository) : ViewModel() {
    val themeConfig: StateFlow<ThemeConfig?> = repository.themeConfig
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = null
        )
    val appLanguage: StateFlow<AppLanguage> = repository.appLanguage
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = AppLanguage.English
        )

    fun onThemeEvent(event: ThemeEvent) {
        viewModelScope.launch {
            when (event) {
                is ThemeEvent.SetMode -> repository.setThemeMode(event.mode)
                is ThemeEvent.SetDarkStyle -> repository.setDarkThemeStyle(event.style)
                is ThemeEvent.SetPalette -> repository.setThemePalette(event.palette)
            }
        }
    }

    fun setAppLanguage(language: AppLanguage) {
        viewModelScope.launch {
            repository.setAppLanguage(language)
        }
    }
}

class ThemeViewModelFactory(private val repository: UserPreferencesRepository) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ThemeViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ThemeViewModel(repository) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class")
    }
}
