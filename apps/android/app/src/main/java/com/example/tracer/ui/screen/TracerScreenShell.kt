package com.example.tracer

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

internal val ScreenOuterPadding: Dp = 16.dp

@Composable
internal fun TracerBottomNavShell(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    snackbarHostState: SnackbarHostState,
    content: @Composable (PaddingValues) -> Unit
) {
    Scaffold(
        snackbarHost = {
            SnackbarHost(hostState = snackbarHostState)
        },
        bottomBar = {
            val navItemColors = NavigationBarItemDefaults.colors(
                selectedIconColor = MaterialTheme.colorScheme.onPrimary,
                selectedTextColor = MaterialTheme.colorScheme.primary,
                unselectedIconColor = MaterialTheme.colorScheme.onSurfaceVariant,
                unselectedTextColor = MaterialTheme.colorScheme.onSurfaceVariant,
                indicatorColor = MaterialTheme.colorScheme.primary
            )

            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                tonalElevation = 0.dp
            ) {
                TracerTabRegistry.entries.forEach { entry ->
                    val tabMeta = entry.meta
                    val isSelected = selectedTab == tabMeta.id
                    val tabTitle = stringResource(tabMeta.titleRes)
                    NavigationBarItem(
                        modifier = if (tabMeta.testTag.isNullOrBlank()) {
                            Modifier
                        } else {
                            Modifier.testTag(tabMeta.testTag)
                        },
                        selected = isSelected,
                        onClick = { onTabSelected(tabMeta.id) },
                        icon = { Icon(tabMeta.icon, contentDescription = tabTitle) },
                        label = {
                            Text(
                                text = tabTitle,
                                style = MaterialTheme.typography.labelMedium.copy(
                                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Medium
                                )
                            )
                        },
                        alwaysShowLabel = true,
                        colors = navItemColors
                    )
                }
            }
        },
        content = content
    )
}
