[English Version](README.en.md) | [中文版本](README.md)

# Time Tracer ![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg) [![Windows Build Matrix](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml) [![Android CI](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml)

<p align="center">
  <img src="ui/branding/master/time_tracer_brand_master_symbol.svg" alt="Time Tracer Logo" width="120" height="120">
  <br>
  <em>Icon designed for camellia2077/time_tracer</em>
</p>

下面是你这段内容的**完整英文翻译版本（已保持技术语义一致，并做了轻微产品化润色，适合直接放 GitHub README）**：

---

**Time Tracer** is an Android-first hierarchical time tracking system. It uses configurable alias mapping to transform user-entered activity tokens into a structured multi-level activity tree, and automatically aggregates durations across parent and child nodes for fine-grained personal time analysis and behavioral review.

The goal of Time Tracer is not simply to record “how long you studied” or “how long you exercised,” but to help users understand where their time actually goes across detailed sub-activities, without introducing significant input overhead. For example, “study” can be decomposed into “computer science” and “mathematics”; “computer science” can further be decomposed into “algorithms,” “computer architecture,” and “computer networks,” while “mathematics” can be decomposed into “calculus” and “linear algebra.” Similarly, “fitness” can be broken down into “strength training” and “cardio.”

The system is built with an **Android + Core Engine** architecture: Android serves as the primary interface for fast daily input and interaction, while the core engine handles parsing, normalization, aggregation, querying, and report generation. Raw records are stored as plain-text logs, while SQLite is used as a query and analytics layer to improve retrieval, aggregation, and report generation efficiency.

---

## Design Principles (Brief)

1. **Low-friction input**  
   Users can input activities using Chinese, English, abbreviations, or custom aliases without manually selecting full hierarchical paths each time.

2. **Hierarchical activity structure**  
   Activities are not flat tags but a tree-like structure similar to a filesystem. Each node can have its own sub-nodes.

3. **Automatic parent-child aggregation**  
   Time recorded at leaf nodes is automatically accumulated into all parent nodes, enabling both fine-grained and high-level analysis simultaneously.

4. **Text as source of truth**  
   All raw records are stored as readable TXT files. Users fully own their data and can back it up, edit it, or migrate it freely. SQLite and reports are derived from these logs.

5. **Unified cross-platform data model**  
   Android, CLI, and reporting tools share the same activity mapping and statistical semantics, reducing format fragmentation across platforms.

---

## What Is Recorded

Each record contains a point in time or a time range, together with the activity name entered by the user. For example:

```text
0613 wake up
0640-1038 algorithms
1930-2030 strength training
```

Activity names may be written in any language, abbreviation, or custom alias. The system resolves them to canonical activity paths, records the duration, and keeps the readable TXT log as the source record. SQLite is derived query and analytics data, not a second source that must be maintained manually.

## Activity Hierarchy and Recording

The activity directory behaves like a folder tree. A normal alias maps input to a leaf activity. A group can also become directly recordable through `group_aliases` while still containing child activities:

```toml
parent = "exercise"

[canonical.strength-training]
group_aliases = ["strength training"]

[canonical.strength-training.squat]
group_aliases = ["squat"]
"front squat" = "front-squat"
```

These inputs record to different canonical paths:

* `strength training` → `exercise_strength-training`
* `squat` → `exercise_strength-training_squat`
* `front squat` → `exercise_strength-training_squat_front-squat`

Time recorded at a child is automatically aggregated into every parent. A record under `exercise_strength-training_squat` is therefore included when querying `exercise_strength-training` or `exercise`. The same data can be inspected at action, category, and top-level summary granularity.

## Can Activities Be Moved or Renamed?

Yes. The CLI and Android configuration flows support structural edits:

* A leaf alias can be promoted to a directly recordable group; promotion itself preserves the existing canonical paths.
* A leaf alias can be moved into an existing category. Its canonical path then changes, for example from `exercise_running` to `exercise_cardio_running`.
* A group record name can be renamed, or an additional record name can be added.

Moving a leaf or renaming a group record name affects historical data. The system updates the canonical TOML, replaces the corresponding canonical activity tokens in TXT, and rebuilds the database; the active data is replaced only after all steps succeed. Adding a group record name affects future input only, so historical TXT and the database do not need to change. Moving a group together with all of its descendants is not currently supported as one operation.

## How Data Is Displayed

Queries and reports aggregate by canonical activity path rather than treating different input aliases as separate activities. Reports provide:

* total recorded time, recorded days, and activity counts for the selected period;
* a hierarchical activity breakdown in which parent durations include child durations;
* daily, weekly, monthly, yearly, and custom-range summaries;
* Markdown, LaTeX, and Typst exports. Markdown supports English, Chinese, and Japanese text.

Thus, `strength training`, `strength`, and any other aliases that resolve to the same canonical path appear as one activity node in queries and reports. TOML defines aliases and hierarchy; detailed configuration and migration constraints are documented under `docs/time_tracer/core/capabilities/config/`.

## Android Query Displays

Android is currently the primary development entry point. Query results are presented in the following user-facing forms:

* **Timeline**: A day query can show that day's activities in chronological order. Each item shows its start time, end time, activity path, duration, and remark. The path is rendered as a top-level category followed by nested activities. The Timeline also supports editing the day remark and activity remarks.
* **Tree**: A selected day, week, month, year, recent-period, or custom-range query can return an activity tree. Nodes are expanded by default, and a node with children can be expanded or collapsed by tapping it. Each node shows its canonical name, relative path, aggregated duration, and a progress bar for its share of the current tree result; child percentages are calculated relative to their parent. Results can be sorted by duration in ascending or descending order.
* **Activity composition charts**: The same activity tree can be rendered as a Pie chart, Horizontal Bar chart, or Treemap. The chart can measure either duration or activity frequency.

Activity composition charts support path-by-path drill-down. Tapping an activity that has children enters the next level for that activity, in the same way as opening a folder; the current path is shown and a control is provided to return to the previous level. At each level, the chart outputs only the selected node's direct children and calculates their duration or frequency shares within that level. The percentage is therefore the composition of the current node, not a fixed global percentage. The legend and selected item show the corresponding value and percentage.




### Core Components

* **`time_tracer_cli` (C++23)**: The core command-line application. It uses a pipeline pattern to process raw text logs and offers efficient SQLite-based queries and multi-format report exports (Markdown, LaTeX, Typst).
* **`graph_generator` (Python)**: A data visualization tool that reads the database and generates dynamic charts like timelines and heatmaps.
* **`log_generator` (C++)**: A helper utility for generating standardized test logs.
---

## 🚀 Quick Start

### 1. Dependencies

* **C++ Components (`time_tracer_cli`)**:
    * **MSYS2 UCRT64** (Recommended for Windows)
    * **CMake** >= 3.25 (C++23 support)
    * **Compiler**: Clang 16+ or GCC 13+
    * **Libraries**: SQLite3, nlohmann/json, toml++
* **Python Component (`graph_generator`)**:
    * **Python** >= 3.8, Matplotlib

### 2. Build Guide

We provide automated build scripts that compile the core application and runtime deliverables in one go.

➡️ **For detailed steps, see: [Build Guide](docs/time_tracer/guides/build_guide.md)**

### 3. Usage Examples

**Example 1: Automated Ingestion Pipeline (Blink)**
(Validate, Convert, Link, and Persist in one step)

```bash
# Ingest all raw logs from the target_logs directory
time_tracer_cli blink -a "path/to/target_logs"
```

**Example 2: Query Data Records**

```bash
# List all days recorded in 2026
time_tracer_cli query data days --year 2026
```

**Example 3: Export Formatted Reports**

```bash
# Export the weekly report for 2026-W05 as Markdown
time_tracer_cli export week 2026-W05 -f md
```

---

## 📚 Documentation

The documentation has been reorganized for better accessibility:

```text
docs/time_tracer/
├── design/                 # Architecture & Core Logic
│   ├── architecture.md     # Clean Architecture layers
│   └── system_design.md    # Design philosophy & data flow
├── guides/                 # Manuals & Configuration
│   ├── build_guide.md      # Build & setup steps
│   └── cli_query_guide.md  # Comprehensive CLI query reference
└── workflows/              # Process Overview
    └── workflow.md         # Full logic flow diagrams
```

---

## Developers & Acknowledgements

### Lead Developer
* **[camellia2077](https://github.com/camellia2077)**: Project creator.

### AI Collaborators
Special thanks to the following AI models for their core assistance in coding, architectural design, and documentation optimization:
* **Google**: Gemini 2.5 Pro, 3 Pro, 3 Flash, 3.1 Pro
* **Anthropic**: Claude 4.5 Opus
* **OpenAI**: GPT-5.2 Codex, 5.3 Codex

---

## Disclaimer

This software is intended for use as a personal efficiency management tool only. Any use of this software in violation of local laws and regulations is strictly prohibited. The developer does not endorse, participate in, or assume any responsibility for the consequences of any third party using this software for political propaganda.

---

## License & Open Source Libraries

This repository's own source code is licensed under **Apache License 2.0** (see `LICENSE`).
Third-party dependencies remain under their respective licenses.

### Core and Tools

* **[SQLite](https://www.sqlite.org/)**: Embedded database (Public Domain).
* **[nlohmann/json](https://github.com/nlohmann/json)**: JSON parsing (MIT).
* **[tomlplusplus](https://github.com/marzer/tomlplusplus)**: TOML configuration (MIT).
* **[libsodium](https://github.com/jedisct1/libsodium)**: Cryptography library (planned for `tracer_core` encrypted export/share capability) (ISC License).
* **[Apache ECharts](https://echarts.apache.org/)**: Used by Windows CLI `report-chart` single-file HTML chart rendering (Line/Bar/Pie/Heatmap-Year/Heatmap-Month) (Apache License 2.0).
* **[Matplotlib](https://matplotlib.org/)**: Plotting engine (BSD-style license).

### Windows Rust CLI (`apps/cli/windows/rust`)

* **[clap](https://github.com/clap-rs/clap)**: Rust CLI argument parser and subcommand framework (MIT or Apache License 2.0).
* **[thiserror](https://github.com/dtolnay/thiserror)**: Rust error type derive helper (MIT or Apache License 2.0).
* **[libloading](https://github.com/nagisa/rust_libloading)**: Dynamic library loading (e.g., runtime DLL) (ISC License).
* **[serde](https://github.com/serde-rs/serde)**: Serialization/deserialization framework (MIT or Apache License 2.0).
* **[serde_json](https://github.com/serde-rs/json)**: JSON handling (MIT or Apache License 2.0).
* **[toml](https://github.com/toml-rs/toml)**: TOML parser (MIT or Apache License 2.0).

Dependency versions are managed in:
* `apps/cli/windows/rust/Cargo.toml`
 
### Android App (`apps/android`)

* **[AndroidX / Jetpack Compose family](https://github.com/androidx/androidx)**  
  Includes `core-ktx`, `lifecycle-*`, `activity-compose`, `compose-*`, `datastore-preferences`, and AndroidX test libraries used by this app.  
  **License**: Apache License 2.0.
* **[Material Components for Android](https://github.com/material-components/material-components-android)** (`com.google.android.material:material`)  
  **License**: Apache License 2.0.
* **[Multiplatform Markdown Renderer](https://github.com/mikepenz/multiplatform-markdown-renderer)** (`com.mikepenz:multiplatform-markdown-renderer-m3`)  
  **License**: Apache License 2.0.
* **[JUnit 4](https://github.com/junit-team/junit4)** (`junit:junit`, test-only dependency)  
  **License**: Eclipse Public License 1.0 (EPL-1.0).

Dependency versions are managed in:
* `apps/android/gradle/libs.versions.toml`
