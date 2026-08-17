package com.example.tracer

import androidx.compose.foundation.ScrollState
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.requiredWidth
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.rememberTextMeasurer
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

/**
 * A fixed-width, horizontally scrollable text tab row that shows its initial selection directly.
 *
 * Material's scrollable tab rows animate their selected item into view after layout. That is useful
 * for in-place selection changes, but makes a restored selection visibly shift when a screen opens.
 * This component seeds [ScrollState] with the final offset instead and leaves later scrolling to the
 * user.
 */
@Composable
internal fun StaticScrollableTextTabRow(
    labels: List<String>,
    selectedIndex: Int,
    onSelectedIndexChange: (Int) -> Unit,
    modifier: Modifier = Modifier,
    indicatorModifier: Modifier = Modifier,
    tabWidth: Dp = 90.dp,
    textStyle: TextStyle = MaterialTheme.typography.titleSmall,
    indicatorHorizontalPadding: Dp = 8.dp,
    indicatorMinimumWidth: Dp = 32.dp
) {
    if (labels.isEmpty()) return

    val selectedTabIndex = selectedIndex.coerceIn(labels.indices)
    val textMeasurer = rememberTextMeasurer()
    val density = LocalDensity.current
    val selectedLabelWidthPx = textMeasurer.measure(
        text = AnnotatedString(labels[selectedTabIndex]),
        style = textStyle
    ).size.width
    val selectedIndicatorWidth = with(density) { selectedLabelWidthPx.toDp() }
        .plus(indicatorHorizontalPadding * 2)
        .coerceAtLeast(indicatorMinimumWidth)
        .coerceAtMost(tabWidth)

    BoxWithConstraints(
        modifier = modifier
            .fillMaxWidth()
            .height(48.dp)
    ) {
        val viewportWidthPx = with(density) { maxWidth.roundToPx() }
        val tabWidthPx = with(density) { tabWidth.roundToPx() }
        val contentWidthPx = tabWidthPx * labels.size
        val initialScrollOffset = (
            selectedTabIndex * tabWidthPx + tabWidthPx / 2 - viewportWidthPx / 2
            ).coerceIn(0, (contentWidthPx - viewportWidthPx).coerceAtLeast(0))
        val scrollState = remember(viewportWidthPx) { ScrollState(initialScrollOffset) }

        Row(
            modifier = Modifier
                .fillMaxHeight()
                .horizontalScroll(scrollState)
        ) {
            labels.forEachIndexed { index, label ->
                val selected = index == selectedTabIndex
                Box(
                    modifier = Modifier
                        .requiredWidth(tabWidth)
                        .fillMaxHeight(),
                    contentAlignment = Alignment.BottomCenter
                ) {
                    Tab(
                        selected = selected,
                        onClick = { onSelectedIndexChange(index) },
                        modifier = Modifier.fillMaxSize(),
                        text = {
                            Text(
                                text = label,
                                style = textStyle,
                                maxLines = 1,
                                softWrap = false
                            )
                        }
                    )
                    if (selected) {
                        Box(
                            modifier = Modifier
                                .requiredWidth(selectedIndicatorWidth)
                                .height(3.dp)
                                .background(
                                    color = MaterialTheme.colorScheme.primary,
                                    shape = RoundedCornerShape(3.dp)
                                )
                                .then(indicatorModifier)
                        )
                    }
                }
            }
        }
        HorizontalDivider(modifier = Modifier.align(Alignment.BottomCenter))
    }
}
