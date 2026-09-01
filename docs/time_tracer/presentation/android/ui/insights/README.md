# Android Insights UI

## Purpose

Define semantic colors for Timeline, Tree, and Insights visualizations.

## When To Open

- Open this when changing Timeline, Tree, chart, or Insights card presentation.
- Keep Insights semantics independent from Material `primary`, `secondary`, and `tertiary` role names.

## What This Doc Does Not Cover

- Shared app surfaces and component colors; see `../color-system.md`.
- Insights data meaning and query behavior; see `../../reference/insights-presentation.md`.

## Semantic Roles

| Semantic role | Light HEX | Dark HEX | Usage |
| --- | --- | --- | --- |
| Tree root and Timeline activity | `#4F46E5` | `#818CF8` | Main activity and top-level hierarchy |
| Tree child | `#4F46E5` at 70% opacity | `#818CF8` at 70% opacity | Secondary hierarchy |
| Tree node | `#4F46E5` | `#818CF8` | Expand/collapse and hierarchy markers |
| Tree progress accent | `#2563EB` | `#60A5FA` | Shared by the Tree progress fill and percentage label |
| Timeline duration | `#0284C7` | `#38BDF8` | Timeline duration text and duration emphasis |
| Timeline track | `#E2E8F0` | `#334155` | Unfilled activity track |
| Insights card | `#FFFFFF` | `#0F172A` | Timeline and Tree card surfaces |
| Insights low-emphasis surface | `#F8FAFC` | `#1E293B` | Gaps and low-emphasis regions |
| Comparison increase | `#1A7F37` | `#3FB950` | Positive period-over-period changes; always paired with an up arrow and explicit text |
| Comparison decrease | `#CF222E` | `#F85149` | Negative period-over-period changes; always paired with a down arrow and explicit text |
| Comparison neutral | `#57606A` | `#8C959F` | Unchanged values; always paired with a neutral icon and explicit text |

## Material 3 Role Mapping

Insights keeps semantic names separate from Material 3 role names, but the
selected palette currently supplies them through this mapping:

| Material 3 role | Insights semantic role | Visible usage |
| --- | --- | --- |
| `primary` | Tree hierarchy | Activity names, root/child hierarchy, and expand/collapse markers |
| `secondary` | Tree progress | Hierarchy progress fill and percentage label |
| `tertiary` | Secondary analytical emphasis | Timeline duration text, chart reference lines, selected chart bars/points, and selected Heatmap cell outline |
| `outlineVariant` | Timeline track | Unfilled timeline/chart tracks and structural chart lines |
| `surfaceVariant` | Gap / low-emphasis surface | Empty or low-emphasis regions between activity content |

The mapping is implemented by `ThemeColorTokens.toInsightsColorTokens()`.
Changing a palette's `tertiary` therefore changes duration and chart-selection
emphasis, but does not change the Tree progress bar. Tree progress follows
`secondary`.

The following are current examples of `tertiary` usage, not an exhaustive
component list:

- Records: duration text.
- Bar Chart: average line and selected current-period bar.
- Line Chart: average line and selected point.
- Heatmap: selected cell outline.

When a vertical Bar Chart has period comparison enabled, the comparison
period's bars are intentionally rendered with `primary` at lower opacity
(`0.35`) while current-period bars use `primary` at higher opacity (`0.75`).
Those comparison bars do not use `tertiary`; `tertiary` remains available for
the average line and selected current-period bar.

These uses represent secondary analytical emphasis, reference information, or
focus/selection states rather than status colors. A future use should follow
the same meaning. It should remain visually distinct from the hierarchy
(`primary`), Tree progress (`secondary`), neutral surfaces, and supporting
text. If a new component uses this role, document its example here and verify
that its Light and Dark values remain readable on the active surface.

The Settings tab's **Insights display** section lets users choose the color pair
used by Activities Overview comparisons: green/red (default), red/green, theme
accent with neutral-gray decrease (`#9CA3AF`), or blue/orange (`#2563EB` /
`#F97316`). This setting changes only the increase/decrease colors; localized
direction text retains its established meaning.

Users can also choose the comparison indicator independently: direction arrows
(default), trend lines, or plus/minus signs. The neutral indicator remains a
minus icon, and localized direction text remains present for every style.

## Tree Progress Bar

The Tree progress bar represents a node's share of the total Tree duration. It is a Tree-specific analytical signal and must remain visually distinct from hierarchy labels and neutral card surfaces.

| Element | Light HEX | Dark HEX | Requirement |
| --- | --- | --- | --- |
| Progress fill and percentage label | `#2563EB` | `#60A5FA` | Use one shared Tree-specific blue accent for both elements |
| Progress track | `#E2E8F0` | `#334155` | Use a neutral track; never use the progress accent for the empty portion |
| Percentage label | `#2563EB` | `#2563EB` | Keep the percentage more prominent than supporting text |
| Tree card surface | `#FFFFFF` | `#0F172A` | Keep the card neutral; do not tint the full card with the progress color |

### Future Theme Requirements

- A new theme may replace the HEX values, but it must preserve the four semantic roles: fill, track, percentage label, and card surface.
- The Tree progress fill and percentage label must reuse one shared accent color and one implementation token.
- The progress fill must use the selected theme's progress accent and must be distinguishable from the hierarchy color when the palette defines separate roles.
- The percentage label must remain readable at the existing small text size and must have stronger emphasis than secondary text.
- The empty track and Tree card must remain neutral and must not inherit the progress hue.
- Do not use green, teal, or success-state colors for the Tree progress bar unless the product meaning changes from duration share to status.
- Comparison increase, decrease, and neutral are separate semantic roles. The selected comparison color scheme controls increase/decrease values; arrows and localized change text remain required so the meaning never depends on color alone.
- When adding a theme, document the replacement HEX values in this table before changing the implementation.


## Usage Rules

- Use the primary activity color for hierarchy and activity identity.
- Use the progress accent only for progress, duration, or completion proportion.
- Keep Insights cards, tracks, gaps, and large fills neutral.
- Do not use the secondary UI color for Tree or Timeline semantics.
- Prefer opacity and layout hierarchy before adding another hue.
- In period comparison, show the shared comparison range once at the top of Overview. Put each metric or activity node's change on its own second line in the form: arrow, explicit direction text, signed absolute value, and percentage when applicable.
- All formal colors in this document are recorded as HEX values.

## Theme Mapping

Insights roles follow the selected UI palette while keeping the same semantic responsibilities:

| Semantic role | Indigo light | Indigo dark | Grey light | Grey dark | Green light | Green dark | Orange light | Orange dark | Rose light | Rose dark | Yellow light | Yellow dark | Parchment fixed | Snowfield fixed | Blueprint fixed | Newsprint fixed | Ink Wash fixed | Kraft fixed | Linen fixed |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Tree hierarchy and activity | `#4F46E5` | `#818CF8` | `#3F3F46` | `#D4D4D8` | `#15803D` | `#86EFAC` | `#C2410C` | `#FDBA74` | `#BE123C` | `#FDA4AF` | `#CA8A04` | `#FDE68A` | `#9E1B1B` | `#4C566A` | `#8FD3FF` | `#30343B` | `#263238` | `#3E3025` | `#5B5147` |
| Tree progress and percentage | `#2563EB` | `#60A5FA` | `#71717A` | `#A1A1AA` | `#16A34A` | `#4ADE80` | `#EA580C` | `#FB923C` | `#E11D48` | `#FB7185` | `#EAB308` | `#FACC15` | `#C78C25` | `#2E3440` | `#4EA5D9` | `#A33F3F` | `#B23A2B` | `#9A5B2F` | `#8B6F52` |
| Timeline duration | `#0284C7` | `#38BDF8` | `#4ADE80` | `#BBF7D0` | `#F97316` | `#FDBA74` | `#F43F5E` | `#FDA4AF` | `#FACC15` | `#FEF08A` | `#C78C25` | `#4C566A` | `#B7E3FF` | `#6A7078` | `#7B5E57` | `#526B45` |
| Track | `#E2E8F0` | `#334155` | `#E4E4E7` | `#3F3F46` | `#E2E8F0` | `#365C43` | `#E2E8F0` | `#334155` | `#E2E8F0` | `#334155` | `#E2E8F0` | `#334155` | `#D7B987` | `#C8D0DC` | `#2C5C78` | `#C7BFB1` | `#C9C6BC` | `#B18A59` | `#C9BFB2` |
| Comparison increase | `#1A7F37` | `#3FB950` | `#1A7F37` | `#3FB950` | `#1A7F37` | `#3FB950` | `#1A7F37` | `#3FB950` | `#1A7F37` | `#3FB950` | `#1A7F37` | `#3FB950` | `#1A7F37` | `#1A7F37` | `#1A7F37` | `#1A7F37` | `#1A7F37` | `#1A7F37` |
| Comparison decrease | `#CF222E` | `#F85149` | `#CF222E` | `#F85149` | `#CF222E` | `#F85149` | `#CF222E` | `#F85149` | `#CF222E` | `#F85149` | `#CF222E` | `#F85149` | `#CF222E` | `#CF222E` | `#CF222E` | `#CF222E` | `#CF222E` | `#CF222E` |
| Comparison neutral | `#57606A` | `#8C959F` | `#57606A` | `#8C959F` | `#57606A` | `#8C959F` | `#57606A` | `#8C959F` | `#57606A` | `#8C959F` | `#57606A` | `#8C959F` | `#57606A` | `#57606A` | `#57606A` | `#57606A` | `#57606A` | `#57606A` |

## Breakdown Bar Layout

- Render all visible bars in the current drill-down level.
- Do not impose a fixed-height inner scroll viewport on the bar chart.
- The surrounding Insights content owns vertical scrolling, so the complete bar list participates in one continuous page scroll.
