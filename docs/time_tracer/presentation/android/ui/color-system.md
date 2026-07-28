# Android UI Color System

## Purpose

Define the shared Android UI color roles and their current HEX values.

Runtime theme colors are defined in `apps/android/app/src/main/java/com/example/tracer/ui/theme/ThemePaletteDefinition.kt`. Report semantic token types are shared through `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/theme/ReportColorTokens.kt`. This document records the same roles as HEX references for design and review.

## When To Open

- Open this before changing the app theme, cards, tabs, inputs, buttons, or shared UI components.
- Update this document whenever a shared UI color changes.

## What This Doc Does Not Cover

- Report-specific semantic usage; see `report/README.md`.
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

The preview is an overview of the theme and does not represent every implementation token. It must not add separate swatches for progress, text, outline, surface, or container colors. The auxiliary swatch may reuse a theme token when appropriate, but it must represent general secondary emphasis rather than Report progress specifically. Report progress remains a Report-specific semantic role.

## Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F1F5F9` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#F8FAFC` | Inputs and low-emphasis areas |
| Surface container | `#F8FAFC` | Shared container surfaces |
| Surface container high | `#F1F5F9` | Stronger neutral grouping |
| Selected container | `#E2E8F0` | Selected tabs and neutral selections |
| Divider and outline | `#E2E8F0` | Borders and separators |
| Strong outline | `#CBD5E1` | Higher-contrast borders |
| Primary | `#4F46E5` | Primary actions and shared emphasis |
| Secondary | `#0D9488` | Secondary UI states outside Report structure |
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
| Selected container | `#334155` | Selected tabs and neutral selections |
| Divider and outline | `#334155` | Borders and separators |
| Strong outline | `#64748B` | Higher-contrast borders |
| Primary | `#818CF8` | Primary actions and shared emphasis |
| Secondary | `#2DD4BF` | Secondary UI states outside Report structure |
| Progress accent | `#38BDF8` | Shared progress and duration emphasis |
| Main text | `#F1F5F9` | Primary readable text |
| Secondary text | `#CBD5E1` | Supporting text |

## Graphite Theme

The Graphite palette keeps the same neutral surface hierarchy while replacing the Indigo emphasis with a restrained graphite and amber pairing.

### Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F4F4F5` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#FAFAFA` | Inputs and low-emphasis areas |
| Selected container | `#E4E4E7` | Selected tabs and neutral selections |
| Primary | `#3F3F46` | Primary actions and shared emphasis |
| Secondary | `#D97706` | Activity and focused secondary states |
| Progress accent | `#D97706` | Progress and duration emphasis |
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

## Teal Theme

The Teal palette uses a deep blue-green primary with a restrained teal accent. It adds a cool, fresh option without introducing a high-saturation color into large surfaces.

### Light Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#F0FDFA` | Main screen background |
| Card and elevated surface | `#FFFFFF` | Cards, dialogs, and elevated content |
| Secondary surface | `#F0FDFA` | Inputs and low-emphasis areas |
| Selected container | `#CCFBF1` | Selected tabs and neutral selections |
| Primary | `#0F766E` | Primary actions and shared emphasis |
| Secondary | `#0D9488` | Activity and focused secondary states |
| Progress accent | `#0D9488` | Progress and duration emphasis |
| Main text | `#0F172A` | Primary readable text |
| Secondary text | `#475569` | Supporting text |

### Dark Theme

| Role | HEX | Usage |
| --- | --- | --- |
| Page background | `#042F2E` | Main screen background |
| Card and elevated surface | `#0F3D3A` | Cards, dialogs, and elevated content |
| Secondary surface | `#134E4A` | Inputs and low-emphasis areas |
| Selected container | `#134E4A` | Selected tabs and neutral selections |
| Primary | `#5EEAD4` | Primary actions and shared emphasis |
| Secondary | `#2DD4BF` | Activity and focused secondary states |
| Progress accent | `#2DD4BF` | Progress and duration emphasis |
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

The Orange palette uses restrained neutral Slate surfaces with orange reserved for primary actions and Report emphasis.

| Mode | Page background | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- |
| Light | `#F1F5F9` | `#C2410C` | `#EA580C` | `#0F172A` |
| Dark | `#020617` | `#FDBA74` | `#FB923C` | `#F1F5F9` |

## Rose Theme

The Rose palette uses restrained neutral Slate surfaces with rose reserved for primary actions and Report emphasis.

| Mode | Page background | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- |
| Light | `#F1F5F9` | `#BE123C` | `#E11D48` | `#0F172A` |
| Dark | `#020617` | `#FDA4AF` | `#FB7185` | `#F1F5F9` |

## Amber Theme

The Amber palette uses restrained neutral Slate surfaces with amber reserved for primary actions and Report emphasis.

| Mode | Page background | Primary | Progress accent | Main text |
| --- | --- | --- | --- | --- |
| Light | `#F1F5F9` | `#B45309` | `#D97706` | `#0F172A` |
| Dark | `#020617` | `#FCD34D` | `#FBBF24` | `#F1F5F9` |

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

The Ink Wash palette uses rice-paper surfaces, ink-black hierarchy, and cinnabar report emphasis.

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

## Usage Rules

- Keep page backgrounds, cards, inputs, and large containers in the neutral roles.
- Use the primary color for actions and shared emphasis.
- Use the progress accent for progress or duration only.
- Do not introduce a new color for a component until an existing role cannot express the required state clearly.
- Keep formal documentation in HEX; implementation aliases may use any readable code identifier.
