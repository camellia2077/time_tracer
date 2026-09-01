package com.example.tracer

import androidx.compose.animation.core.animateDpAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.clickable
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.remember
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
import com.example.tracer.ui.components.FullscreenOverlayHost
import com.example.tracer.ui.components.LocalFullscreenOverlayHost

internal val ScreenOuterPadding: Dp = 16.dp

private val FloatingBottomNavHeight: Dp = 60.dp
private val FloatingBottomNavHorizontalPadding: Dp = 12.dp
private val FloatingBottomNavBottomPadding: Dp = 20.dp
private val FloatingBottomNavSnackbarGap: Dp = 12.dp
private val FloatingBottomNavItemWidth: Dp = 88.dp
private val FloatingBottomNavSelectedIndicatorWidth: Dp = 68.dp
private val FloatingBottomNavSelectedIndicatorHeight: Dp = 46.dp
private val FloatingBottomNavSelectedIndicatorTopPadding: Dp = 7.dp
private val FloatingBottomNavShape = RoundedCornerShape(percent = 50)

@Composable
internal fun TracerBottomNavShell(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    snackbarHostState: SnackbarHostState,
    content: @Composable (PaddingValues) -> Unit
) {
    val bottomNavSafePadding = bottomNavSafePadding()
    val fullscreenOverlayHost = remember { FullscreenOverlayHost() }
    CompositionLocalProvider(LocalFullscreenOverlayHost provides fullscreenOverlayHost) {
        FullscreenPageHost {
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
                fullscreenOverlayHost.content?.invoke()
            }
        }
    }
}

@Composable
private fun BoxScope.TracerFloatingBottomNavigation(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    bottomPadding: Dp
) {
    val selectedTabIndex = TracerTabRegistry.indexOf(selectedTab)
    val selectedIndicatorOffset = FloatingBottomNavItemWidth * selectedTabIndex +
        (FloatingBottomNavItemWidth - FloatingBottomNavSelectedIndicatorWidth) / 2
    val animatedSelectedIndicatorOffset = animateDpAsState(
        targetValue = selectedIndicatorOffset,
        animationSpec = tween(durationMillis = 140),
        label = "bottom_nav_indicator"
    )
    val selectedIndicatorColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.14f)
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
        shadowElevation = 2.dp
    ) {
        Box(
            modifier = Modifier
                .width(FloatingBottomNavItemWidth * TracerTabRegistry.entries.size)
                .height(FloatingBottomNavHeight)
        ) {
            Box(
                modifier = Modifier
                    .align(Alignment.TopStart)
                    .offset(
                        x = animatedSelectedIndicatorOffset.value,
                        y = FloatingBottomNavSelectedIndicatorTopPadding
                    )
                    .width(FloatingBottomNavSelectedIndicatorWidth)
                    .height(FloatingBottomNavSelectedIndicatorHeight)
                    .background(selectedIndicatorColor, FloatingBottomNavShape)
            )
            Row(modifier = Modifier.fillMaxSize()) {
                TracerTabRegistry.entries.forEach { entry ->
                    val tabMeta = entry.meta
                    val isSelected = selectedTab == tabMeta.id
                    val tabTitle = stringResource(tabMeta.titleRes)
                    val interactionSource = remember(tabMeta.id) { MutableInteractionSource() }
                    Box(
                        modifier = Modifier
                            .width(FloatingBottomNavItemWidth)
                            .fillMaxSize()
                            .then(
                                if (tabMeta.testTag.isNullOrBlank()) {
                                    Modifier
                                } else {
                                    Modifier.testTag(tabMeta.testTag)
                                }
                            )
                            .clickable(
                                interactionSource = interactionSource,
                                indication = null,
                                onClick = { onTabSelected(tabMeta.id) }
                            )
                    ) {
                        Column(
                            modifier = Modifier.align(Alignment.Center),
                            horizontalAlignment = Alignment.CenterHorizontally,
                            verticalArrangement = Arrangement.spacedBy(2.dp)
                        ) {
                            Icon(
                                imageVector = if (isSelected) {
                                    tabMeta.selectedIcon
                                } else {
                                    tabMeta.unselectedIcon
                                },
                                contentDescription = tabTitle,
                                tint = if (isSelected) {
                                    MaterialTheme.colorScheme.primary
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
                                    MaterialTheme.colorScheme.primary
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
                    }
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
