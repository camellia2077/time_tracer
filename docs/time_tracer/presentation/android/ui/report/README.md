# Android Report UI

## Purpose

Define semantic colors for Timeline, Tree, and Report visualizations.

## When To Open

- Open this when changing Timeline, Tree, chart, or Report card presentation.
- Keep Report semantics independent from Material `primary`, `secondary`, and `tertiary` role names.

## What This Doc Does Not Cover

- Shared app surfaces and component colors; see `../color-system.md`.
- Report data meaning and query behavior; see `../../reference/report-presentation.md`.

## Semantic Roles

| Semantic role | Light HEX | Dark HEX | Usage |
| --- | --- | --- | --- |
| Tree root and Timeline activity | `#4F46E5` | `#818CF8` | Main activity and top-level hierarchy |
| Tree child | `#4F46E5` at 70% opacity | `#818CF8` at 70% opacity | Secondary hierarchy |
| Tree node | `#4F46E5` | `#818CF8` | Expand/collapse and hierarchy markers |
| Tree progress accent | `#2563EB` | `#60A5FA` | Shared by the Tree progress fill and percentage label |
| Timeline duration | `#0284C7` | `#38BDF8` | Timeline duration text and duration emphasis |
| Timeline track | `#E2E8F0` | `#334155` | Unfilled activity track |
| Report card | `#FFFFFF` | `#0F172A` | Timeline and Tree card surfaces |
| Report low-emphasis surface | `#F8FAFC` | `#1E293B` | Gaps and low-emphasis regions |

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
- When adding a theme, document the replacement HEX values in this table before changing the implementation.


## Usage Rules

- Use the primary activity color for hierarchy and activity identity.
- Use the progress accent only for progress, duration, or completion proportion.
- Keep Report cards, tracks, gaps, and large fills neutral.
- Do not use the secondary UI color for Tree or Timeline semantics.
- Prefer opacity and layout hierarchy before adding another hue.
- All formal colors in this document are recorded as HEX values.

## Theme Mapping

Report roles follow the selected UI palette while keeping the same semantic responsibilities:

| Semantic role | Indigo light | Indigo dark | Graphite light | Graphite dark | Teal light | Teal dark | Orange light | Orange dark | Rose light | Rose dark | Amber light | Amber dark | Parchment fixed | Snowfield fixed | Blueprint fixed | Newsprint fixed | Ink Wash fixed | Kraft fixed |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Tree hierarchy and activity | `#4F46E5` | `#818CF8` | `#3F3F46` | `#D4D4D8` | `#0F766E` | `#5EEAD4` | `#C2410C` | `#FDBA74` | `#BE123C` | `#FDA4AF` | `#B45309` | `#FCD34D` | `#9E1B1B` | `#4C566A` | `#8FD3FF` | `#30343B` | `#263238` | `#3E3025` |
| Tree progress and percentage | `#2563EB` | `#60A5FA` | `#D97706` | `#FBBF24` | `#0D9488` | `#2DD4BF` | `#EA580C` | `#FB923C` | `#E11D48` | `#FB7185` | `#D97706` | `#FBBF24` | `#C78C25` | `#2E3440` | `#4EA5D9` | `#A33F3F` | `#B23A2B` | `#9A5B2F` |
| Timeline duration | `#0284C7` | `#38BDF8` | `#D97706` | `#FBBF24` | `#0D9488` | `#2DD4BF` | `#F97316` | `#FDBA74` | `#F43F5E` | `#FDA4AF` | `#F59E0B` | `#FCD34D` | `#C78C25` | `#4C566A` | `#B7E3FF` | `#6A7078` | `#7B5E57` | `#526B45` |
| Track | `#E2E8F0` | `#334155` | `#E4E4E7` | `#3F3F46` | `#CCFBF1` | `#134E4A` | `#E2E8F0` | `#334155` | `#E2E8F0` | `#334155` | `#E2E8F0` | `#334155` | `#D7B987` | `#C8D0DC` | `#2C5C78` | `#C7BFB1` | `#C9C6BC` | `#B18A59` |

## Breakdown Bar Layout

- Render all visible bars in the current drill-down level.
- Do not impose a fixed-height inner scroll viewport on the bar chart.
- The surrounding Report content owns vertical scrolling, so the complete bar list participates in one continuous page scroll.
