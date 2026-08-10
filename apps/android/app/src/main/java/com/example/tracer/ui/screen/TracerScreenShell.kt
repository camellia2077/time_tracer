package com.example.tracer

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.background
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Icon
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.LocalRippleConfiguration
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

internal val ScreenOuterPadding: Dp = 16.dp

private val FloatingBottomNavHeight: Dp = 60.dp
private val FloatingBottomNavHorizontalPadding: Dp = 12.dp
private val FloatingBottomNavBottomPadding: Dp = 20.dp
private val FloatingBottomNavSnackbarGap: Dp = 12.dp
private val FloatingBottomNavItemWidth: Dp = 88.dp
private val FloatingBottomNavSelectedIndicatorHorizontalPadding: Dp = 12.dp
private val FloatingBottomNavSelectedIndicatorVerticalPadding: Dp = 4.dp
private val FloatingBottomNavShape = RoundedCornerShape(percent = 50)

@Composable
internal fun TracerBottomNavShell(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    snackbarHostState: SnackbarHostState,
    content: @Composable (PaddingValues) -> Unit
) {
    val bottomNavSafePadding = bottomNavSafePadding()
    Box(modifier = Modifier.fillMaxSize()) {
        // Do not reserve the floating navigation's footprint here. A bottom padding on this
        // full-screen content container leaves the root Surface exposed as a white/dark strip
        // below the navigation. Vertically scrolling tabs add that space inside their scroll
        // content instead; see tracerTabContentModifier.
        content(WindowInsets.statusBars.asPaddingValues())

        SnackbarHost(
            hostState = snackbarHostState,
            snackbar = { data ->
                val visuals = data.visuals as? TracerSnackbarVisuals
                Surface(
                    modifier = Modifier.fillMaxWidth(),
                    shape = MaterialTheme.shapes.large,
                    color = MaterialTheme.colorScheme.inverseSurface,
                    contentColor = MaterialTheme.colorScheme.inverseOnSurface,
                    tonalElevation = 6.dp,
                    shadowElevation = 6.dp
                ) {
                    // Render the snackbar as two explicit text slots when supporting text is
                    // available. This keeps Record success visually stable across Material
                    // versions and avoids relying on embedded newline behavior in a single Text.
                    Column(
                        verticalArrangement = Arrangement.spacedBy(2.dp),
                        modifier = Modifier.padding(horizontal = 16.dp, vertical = 14.dp)
                    ) {
                        Text(
                            text = data.visuals.message,
                            style = MaterialTheme.typography.bodyMedium.copy(
                                color = MaterialTheme.colorScheme.inverseOnSurface,
                                fontSize = 18.sp,
                                lineHeight = 24.sp
                            )
                        )
                        if (!visuals?.supportingText.isNullOrBlank()) {
                            Text(
                                text = visuals.supportingText.orEmpty(),
                                style = MaterialTheme.typography.bodySmall.copy(
                                    color = MaterialTheme.colorScheme.inverseOnSurface,
                                    fontSize = 16.sp,
                                    lineHeight = 22.sp
                                )
                            )
                        }
                    }
                }
            },
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .fillMaxWidth()
                .padding(
                    start = FloatingBottomNavHorizontalPadding,
                    end = FloatingBottomNavHorizontalPadding,
                    bottom = FloatingBottomNavHeight +
                        bottomNavSafePadding +
                        FloatingBottomNavSnackbarGap
                )
        )

        TracerFloatingBottomNavigation(
            selectedTab = selectedTab,
            onTabSelected = onTabSelected,
            bottomPadding = bottomNavSafePadding
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun BoxScope.TracerFloatingBottomNavigation(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    bottomPadding: Dp
) {
    val navItemColors = NavigationBarItemDefaults.colors(
        selectedIconColor = MaterialTheme.colorScheme.onPrimary,
        selectedTextColor = MaterialTheme.colorScheme.primary,
        unselectedIconColor = MaterialTheme.colorScheme.onSurfaceVariant,
        unselectedTextColor = MaterialTheme.colorScheme.onSurfaceVariant,
        indicatorColor = Color.Transparent
    )

    val navContainerColor = MaterialTheme.colorScheme.surfaceContainer
    Surface(
        modifier = Modifier
            .align(Alignment.BottomCenter)
            .wrapContentWidth()
            .padding(
                start = FloatingBottomNavHorizontalPadding,
                end = FloatingBottomNavHorizontalPadding,
                bottom = bottomPadding
            ),
        shape = FloatingBottomNavShape,
        color = navContainerColor,
        tonalElevation = 0.dp,
        shadowElevation = 6.dp
    ) {
        NavigationBar(
            modifier = Modifier
                .width(FloatingBottomNavItemWidth * TracerTabRegistry.entries.size)
                .height(FloatingBottomNavHeight),
            windowInsets = WindowInsets(0, 0, 0, 0),
            containerColor = navContainerColor,
            tonalElevation = 0.dp
        ) {
            TracerTabRegistry.entries.forEach { entry ->
                val tabMeta = entry.meta
                val isSelected = selectedTab == tabMeta.id
                val tabTitle = stringResource(tabMeta.titleRes)
                CompositionLocalProvider(LocalRippleConfiguration provides null) {
                    NavigationBarItem(
                        modifier = if (tabMeta.testTag.isNullOrBlank()) {
                            Modifier
                        } else {
                            Modifier.testTag(tabMeta.testTag)
                        },
                        selected = isSelected,
                        onClick = { onTabSelected(tabMeta.id) },
                        icon = {
                            Column(
                                modifier = Modifier
                                    .background(
                                        color = if (isSelected) {
                                            MaterialTheme.colorScheme.primary
                                        } else {
                                            Color.Transparent
                                        },
                                        shape = FloatingBottomNavShape
                                    )
                                    .padding(
                                        horizontal = if (isSelected) {
                                            FloatingBottomNavSelectedIndicatorHorizontalPadding
                                        } else {
                                            0.dp
                                        },
                                        vertical = if (isSelected) {
                                            FloatingBottomNavSelectedIndicatorVerticalPadding
                                        } else {
                                            0.dp
                                        }
                                    ),
                                horizontalAlignment = Alignment.CenterHorizontally,
                                verticalArrangement = Arrangement.spacedBy(2.dp)
                            ) {
                                Icon(
                                    imageVector = tabMeta.icon,
                                    contentDescription = tabTitle,
                                    tint = if (isSelected) {
                                        MaterialTheme.colorScheme.onPrimary
                                    } else {
                                        MaterialTheme.colorScheme.onSurfaceVariant
                                    }
                                )
                                Text(
                                    text = tabTitle,
                                    modifier = if (tabMeta.testTag.isNullOrBlank()) {
                                        Modifier
                                    } else {
                                        Modifier.testTag("${tabMeta.testTag}_label")
                                    },
                                    color = if (isSelected) {
                                        MaterialTheme.colorScheme.onPrimary
                                    } else {
                                        MaterialTheme.colorScheme.onSurfaceVariant
                                    },
                                    maxLines = 1,
                                    softWrap = false,
                                    overflow = TextOverflow.Ellipsis,
                                    style = MaterialTheme.typography.labelMedium.copy(
                                        fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Medium
                                    )
                                )
                            }
                        },
                        label = null,
                        alwaysShowLabel = false,
                        colors = navItemColors
                    )
                }
            }
        }
    }
}

@Composable
private fun bottomNavSafePadding(): Dp {
    val navigationBottomPadding = WindowInsets.navigationBars
        .asPaddingValues()
        .calculateBottomPadding()
    return if (navigationBottomPadding > FloatingBottomNavBottomPadding) {
        navigationBottomPadding
    } else {
        FloatingBottomNavBottomPadding
    }
}

@Composable
internal fun floatingBottomNavScrollPadding(): Dp {
    // Keep the final scrollable item clear of both the floating bar and the system navigation
    // area. This must remain scroll-content padding rather than shell padding, otherwise the
    // reserved space becomes a permanently visible solid background strip.
    return FloatingBottomNavHeight + bottomNavSafePadding()
}
