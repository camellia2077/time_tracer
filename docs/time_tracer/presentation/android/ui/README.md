# Android UI Documentation

## Purpose

Define the visual language and presentation rules for the Android UI.

## When To Open

- Open this when changing colors, typography, reusable components, or Insights presentation.
- Start with `color-system.md` for the current color contract.
- Use `insights/` for Insights-specific visual semantics.

## What This Doc Does Not Cover

- Persisted preference schema
- Runtime data semantics
- Build and validation workflow

## Sections

- Common UI theme:
  - `color-system.md`
- Insights presentation:
  - `insights/README.md`

## Rules

- Document formal colors with HEX values.
- Color names such as Indigo, Slate, Teal, or Sky are implementation labels only; they are not the documentation contract.
- Keep large surfaces neutral and reserve accent colors for interaction, hierarchy, activity, and progress.
