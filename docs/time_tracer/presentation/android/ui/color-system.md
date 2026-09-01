# Android UI Color System

## Purpose

Define the shared Android UI color roles and their current HEX values.

Runtime theme colors are defined in `apps/android/app/src/main/java/com/example/tracer/ui/theme/ThemePaletteDefinition.kt`. Insights semantic token types are shared through `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/theme/InsightsColorTokens.kt`. This document records the same roles as HEX references for design and review.

## When To Open

- Open this before changing the app theme, cards, tabs, inputs, buttons, or shared UI components.
- Update this document whenever a shared UI color changes.

## What This Doc Does Not Cover

- Detailed Insights-specific semantic usage; see `insights/README.md`.
- User preference persistence; see `../specs/preference-storage.md`.

## Design Direction

The shared UI uses a cool blue-gray neutral foundation. Large areas must remain neutral so that text and semantic accents remain easy to read.

## Theme Preview Card

The Theme Palette selector uses a compact three-swatch preview for every theme. The preview must always show exactly these colors, in this order:

| Swatch | Theme role | Meaning |
| --- | --- | --- |
| 1 | Primary | Main emphasis color for actions and hierarchy |
| 2 | Auxiliary | Auxiliary theme color for secondary emphasis |
| 3 | Page background | Theme's primary large-area background |

The preview is an overview of the theme and does not represent every implementation token. It must not add separate swatches for progress, text, outline, surface, or container colors. The auxiliary swatch may reuse a theme token when appropriate, but it must represent general secondary emphasis rather than Insights progress specifically. Insights progress remains an Insights-specific semantic role.

## Material 3 Role Semantics

The app uses the Material 3 color roles as a semantic contract rather than as
three interchangeable accent colors. The same role can be consumed by shared
UI and by an Insights-specific semantic token.

| Material 3 role | App meaning | Main visible uses |
| --- | --- | --- |
| `primary` | Main identity and hierarchy emphasis | Actions, activity identity, Insights hierarchy names and markers |
| `primaryContainer` | Theme-related selected-state fill | Selected options in shared mutually exclusive capsule/segmented controls |
| `secondary` | Secondary identity and tree progress emphasis | Secondary UI states; Insights Tree progress fill and percentage label |
| `tertiary` | Secondary analytical emphasis, reference, and focus states | Insights duration text, chart reference lines, selected data points/bars, and selected Heatmap cell outline |
| `surface`, `surfaceContainer*` | Neutral content layers | Page surfaces, cards, inputs, and grouped containers |
| `outline`, `outlineVariant` | Structural boundaries | Borders, dividers, chart tracks, and input outlines |
| `on*` roles | Content drawn on the corresponding color | Text and icons placed on primary, secondary, tertiary, container, or surface colors |

`tertiary` is intentionally not used as a general card, button, or bottom-tab
color. The examples above are not an exhaustive list: a future component may
use `tertiary` when its meaning is secondary analytical emphasis, a reference
indicator, or a focused/selected state. It should not be used for page
backgrounds, ordinary cards, success/error status, or a semantic progress
indicator that already belongs to `secondary`.

When adding or changing a palette, keep `tertiary` related to the palette's
primary color family or otherwise visually coordinated with it. It should be
distinct enough from `primary` to communicate a different role, readable on
the active surface in both Light and Dark modes, and more noticeable than
supporting text without overpowering the main data. New uses should be added
to the semantic examples in `insights/README.md` only when they introduce a
new user-visible use of this role.

### Selected capsule controls

Shared mutually exclusive capsule controls use
`MaterialTheme.colorScheme.primaryContainer` as their selected fill,
`onPrimaryContainer` for the label, and `primary` for the selected outline.
Every switchable palette should provide a light and dark `primaryContainer`
that is visually related to its own primary color. Grey is the intentional
neutral exception: its primary and primary container remain grayscale.

The selected fill is a low-saturation tint or a darker related shade rather
than the full primary color. This keeps the selected state clear while leaving
the stronger primary, secondary, and tertiary colors available for actions,
progress, and analytical emphasis.

Breakdown Bar tracks use the neutral `surfaceContainerHighest` role for the
portion not covered by chart data. Switchable palettes keep this track neutral
in both modes; theme color is reserved for the data-covered bar itself.

| Palette | Light selected fill | Dark selected fill |
| --- | --- | --- |
| Indigo | `#E0E7FF` | `#3730A3` |
| Purple | `#E9D5FF` | `#581C87` |
| Grey | `#E4E4E7` | `#3F3F46` |
| Green | `#DCFCE7` | `#14532D` |
| Blue | `#DBEAFE` | `#1E40AF` |
| Orange | `#FFEDD5` | `#9A3412` |
| Rose | `#FFE4E6` | `#9F1239` |
| Yellow | `#FEF3C7` | `#713F12` |

## Dark Surface Style

The Settings appearance setting controls the dark-mode surface treatment rather
than the semantic theme colors. Its options are:

| Option | Meaning | Surface treatment |
| --- | --- | --- |
| `Neutral` / 中性灰阶 | Shared neutral dark surfaces | Background and containers use the common neutral gray-black scale |
| `Black` / 纯黑 | Maximum-black surfaces | Background is pure black; cards use a clearly separated near-black surface and containers continue through lighter near-black levels |

The setting affects `background`, `surface`, `surfaceVariant`, and all
`surfaceContainer*` roles. It does not change `primary`, `secondary`,
`tertiary`, `primaryContainer`, chart colors, progress colors, or other
semantic accents. `Neutral` is the default shared surface scale.

## Light Surface Style

The Settings appearance setting exposes two light-mode surface styles for
switchable palettes:

| Option | Surface treatment |
| --- | --- |
| `Neutral` / 中性灰阶 | Current light treatment: `#F1F5F9` page background and white cards |
| `Elevated` / 凸显层级 | `#E2E8F0` page background and white cards, with stronger container separation |

Fixed palettes such as `Parchment` and `Snowfield` do not show this setting,
and the selected light surface style does not affect their colors.

## Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F1F5F9` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#F8FAFC` | Inputs and low-emphasis areas |
| Surface container | `#F8FAFC` | Shared container surfaces |
| Surface container high | `#F1F5F9` | Stronger neutral grouping |
| Selected container | `#E0E7FF` | Selected options in mutually exclusive capsule controls |
| Divider and outline | `#E2E8F0` | Borders and separators |
| Strong outline | `#CBD5E1` | Higher-contrast borders |
| Primary | `#4F46E5` | Primary actions and shared emphasis |
| Secondary | `#0D9488` | Secondary UI states outside Insights structure |
| Progress accent | `#0284C7` | Shared progress and duration emphasis |
| Main text | `#0F172A` | Primary readable text |
| Secondary text | `#475569` | Supporting text |

## Dark Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#020617` | Main screen background |
| Card and elevated surface | `#0F172A` | Cards, dialogs, and elevated content |
| Secondary surface | `#1E293B` | Inputs and low-emphasis areas |
| Surface container | `#1E293B` | Shared container surfaces |
| Surface container high | `#334155` | Stronger neutral grouping |
| Selected container | `#3730A3` | Selected options in mutually exclusive capsule controls |
| Divider and outline | `#334155` | Borders and separators |
| Strong outline | `#64748B` | Higher-contrast borders |
| Primary | `#818CF8` | Primary actions and shared emphasis |
| Secondary | `#2DD4BF` | Secondary UI states outside Insights structure |
| Progress accent | `#38BDF8` | Shared progress and duration emphasis |
| Main text | `#F1F5F9` | Primary readable text |
| Secondary text | `#CBD5E1` | Supporting text |

## Grey Theme

The Grey palette keeps the same neutral surface hierarchy while using a restrained grayscale emphasis.

### Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F4F4F5` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#FAFAFA` | Inputs and low-emphasis areas |
| Selected container | `#E4E4E7` | Selected tabs and neutral selections |
| Primary | `#3F3F46` | Primary actions and shared emphasis |
| Secondary | `#71717A` | Activity and focused secondary states |
| Progress accent | `#71717A` | Progress and duration emphasis |
| Main text | `#18181B` | Primary readable text |
| Secondary text | `#52525B` | Supporting text |

### Dark Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#09090B` | Main screen background |
| Card and elevated surface | `#18181B` | Cards, dialogs, and elevated content |
| Secondary surface | `#27272A` | Inputs and low-emphasis areas |
| Selected container | `#3F3F46` | Selected tabs and neutral selections |
| Primary | `#D4D4D8` | Primary actions and shared emphasis |
| Secondary | `#FBBF24` | Activity and focused secondary states |
| Progress accent | `#FBBF24` | Progress and duration emphasis |
| Main text | `#F4F4F5` | Primary readable text |
| Secondary text | `#A1A1AA` | Supporting text |

## Green Theme

The Green palette uses a conventional green primary with lighter green accents. It keeps large surfaces neutral while reserving green for interaction and Insights emphasis.

### Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F0FDFA` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#F0FDFA` | Inputs and low-emphasis areas |
| Selected container | `#DCFCE7` | Selected options in mutually exclusive capsule controls |
| Primary | `#15803D` | Primary actions and shared emphasis |
| Secondary | `#16A34A` | Activity and focused secondary states |
| Progress accent | `#16A34A` | Progress and duration emphasis |
| Main text | `#0F172A` | Primary readable text |
| Secondary text | `#475569` | Supporting text |

### Dark Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#042F2E` | Main screen background |
| Card and elevated surface | `#0F3D3A` | Cards, dialogs, and elevated content |
| Secondary surface | `#134E4A` | Inputs and low-emphasis areas |
| Selected container | `#14532D` | Selected options in mutually exclusive capsule controls |
| Primary | `#86EFAC` | Primary actions and shared emphasis |
| Secondary | `#4ADE80` | Activity and focused secondary states |
| Progress accent | `#4ADE80` | Progress and duration emphasis |
| Main text | `#F0FDFA` | Primary readable text |
| Secondary text | `#99F6E4` | Supporting text |

## Parchment Theme (Fixed)

The Parchment palette is a fixed warm appearance. It does not change when the system or the app switches between light and dark modes.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#E8E2D0` | Main screen background |
| Card and elevated surface | `#F3EEDC` | Cards, dialogs, and elevated content |
| Secondary surface | `#EDE5D1` | Inputs and low-emphasis areas |
| Primary | `#9E1B1B` | Primary actions and shared emphasis |
| Progress accent | `#C78C25` | Tree progress fill and percentage label |
| Main text | `#382F24` | Primary readable text |
| Secondary text | `#6D5B45` | Supporting text |

## Snowfield Theme (Fixed)

The Snowfield palette is a fixed cool neutral appearance. It does not change when the system or the app switches between light and dark modes.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#E5E9F0` | Main screen background |
| Card and elevated surface | `#ECEFF4` | Cards, dialogs, and elevated content |
| Secondary surface | `#D8DEE9` | Inputs and low-emphasis areas |
| Primary | `#4C566A` | Primary actions and shared emphasis |
| Progress accent | `#2E3440` | Tree progress fill and percentage label |
| Main text | `#2E3440` | Primary readable text |
| Secondary text | `#4C566A` | Supporting text |

## Orange Theme

The Orange palette uses restrained neutral Slate surfaces while using orange
for primary actions, selected capsule fills, and Insights emphasis.

| Mode | Page background | Selected container | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- | --- |
| Light | `#F1F5F9` | `#FFEDD5` | `#C2410C` | `#EA580C` | `#0F172A` |
| Dark | `#020617` | `#9A3412` | `#FDBA74` | `#FB923C` | `#F1F5F9` |

## Rose Theme

The Rose palette uses restrained neutral Slate surfaces while using rose for
primary actions, selected capsule fills, and Insights emphasis.

| Mode | Page background | Selected container | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- | --- |
| Light | `#F1F5F9` | `#FFE4E6` | `#BE123C` | `#E11D48` | `#0F172A` |
| Dark | `#020617` | `#9F1239` | `#FDA4AF` | `#FB7185` | `#F1F5F9` |

## Yellow Theme

The Yellow palette uses restrained neutral Slate surfaces with yellow-gold
accents reserved for primary actions and Insights emphasis.

| Mode | Page background | Selected container | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- | --- |
| Light | `#F4F4F5` | `#FEF3C7` | `#CA8A04` | `#EAB308` | `#18181B` |
| Dark | `#1C1B1F` | `#713F12` | `#FDE68A` | `#FACC15` | `#F4F4F5` |

## Blueprint Theme (Fixed)

The Blueprint palette is a dark engineering-paper skin with a high-contrast cyan-blue writing and chart accent.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#0B1F33` | Main screen background |
| Card and elevated surface | `#12304A` | Cards, dialogs, and elevated content |
| Primary | `#8FD3FF` | Primary actions and shared emphasis |
| Progress accent | `#4EA5D9` | Tree progress fill and percentage label |
| Main text | `#EAF6FF` | Primary readable text |

## Newsprint Theme (Fixed)

The Newsprint palette evokes cool gray newspaper stock with black ink and a restrained print red.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#D9D9D6` | Main screen background |
| Card and elevated surface | `#F1F1ED` | Cards, dialogs, and elevated content |
| Primary | `#30343B` | Primary actions and shared emphasis |
| Progress accent | `#A33F3F` | Tree progress fill and percentage label |
| Main text | `#202124` | Primary readable text |

## Ink Wash Theme (Fixed)

The Ink Wash palette uses rice-paper surfaces, ink-black hierarchy, and cinnabar insights emphasis.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F2EFE6` | Main screen background |
| Card and elevated surface | `#FAF8F2` | Cards, dialogs, and elevated content |
| Primary | `#263238` | Primary actions and shared emphasis |
| Progress accent | `#B23A2B` | Tree progress fill and percentage label |
| Main text | `#263238` | Primary readable text |

## Kraft Theme (Fixed)

The Kraft palette uses a strong brown paper skin with dark ink, leather-brown, and muted green accents.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#C9A875` | Main screen background |
| Card and elevated surface | `#E4C99A` | Cards, dialogs, and elevated content |
| Primary | `#3E3025` | Primary actions and shared emphasis |
| Progress accent | `#9A5B2F` | Tree progress fill and percentage label |
| Main text | `#3E3025` | Primary readable text |

## Linen Theme (Fixed)

The Linen palette evokes the soft neutral tones of linen-bound books and
cloth-covered hardbacks. It remains fixed and does not follow system or app
light/dark mode changes.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#E7E1D8` | Main screen background |
| Card and elevated surface | `#F7F3EC` | Cards, dialogs, and elevated content |
| Secondary surface | `#EDE7DE` | Inputs and low-emphasis areas |
| Primary | `#5B5147` | Primary actions and shared emphasis |
| Progress accent | `#8B6F52` | Tree progress fill and percentage label |
| Main text | `#302A25` | Primary readable text |

## Mint Theme (Fixed)

The Mint palette is a calm, low-saturation green appearance inspired by
lightly tinted paper and fresh botanical colors. It remains fixed and does
not follow system or app light/dark mode changes.

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#E8F5EE` | Main screen background |
| Card and elevated surface | `#F7FCF9` | Cards, dialogs, and elevated content |
| Secondary surface | `#EFF8F2` | Inputs and low-emphasis areas |
| Primary | `#247A57` | Primary actions and shared emphasis |
| Progress accent | `#3B8D6E` | Tree progress fill and percentage label |
| Main text | `#1E3027` | Primary readable text |

## Theme Palette Groups

The palette selector groups themes into `Light/Dark Mode Themes` and `Fixed
Appearance Themes`.

| Group | Light/Dark behavior | Surface-style behavior |
| --- | --- | --- |
| `Light/Dark Mode Themes` / 支持明暗模式的主题 | Follows the selected `Light`, `Dark`, or `System` mode | Shows and applies the matching light or dark surface style |
| `Fixed Appearance Themes` / 固定外观主题 | Does not follow `Light`, `Dark`, or system mode; it keeps one fixed appearance | Does not show or apply light/dark surface styles |

Each group can be expanded or collapsed independently in the palette selector.

## Usage Rules

- Keep page backgrounds, cards, inputs, and large containers in the neutral roles.
- Use the primary color for actions and shared emphasis.
- Use the theme-related `primaryContainer` for selected mutually exclusive capsule controls; pair it with `onPrimaryContainer` text and a `primary` outline.
- Use the progress accent for progress or duration only.
- Do not introduce a new color for a component until an existing role cannot express the required state clearly.
- Keep formal documentation in HEX; implementation aliases may use any readable code identifier.
