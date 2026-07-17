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

## Text Input & Alias Mapping Example

Time Tracer uses Android as its primary interface, but underlying data can be represented as simple, readable TXT logs. Each line acts like a “time subtitle,” describing what happened at a specific time or time range.

For example, events can be recorded in the format `HHMM + activity token` or `HHMM-HHMM + activity token`:

```text
0613 wake up
0634 breakfast
0640-1038 algorithms
1038-1224 linear algebra
1443-1922 multiple integrals // exercises
1930-2030 strength training
````

Here, `wake up`, `algorithms`, `linear algebra`, `multiple integrals`, and `strength training` are user-entered activity tokens. They can be written in any language, abbreviation, or custom format. The system uses alias mapping to resolve these tokens into canonical activity paths.

---

## Daily Activity Example

The corresponding alias child file can be defined as:

```toml
parent = "routine"

[aliases]
"wash face" = "oral-hygiene"
"brush teeth" = "oral-hygiene"
"o" = "oral-hygiene"
```

This configuration means:

* `wash face`, `brush teeth`, and `o` will all be mapped to the same canonical path `routine_oral-hygiene`
* The left side represents the user-input token (which can be short for fast entry)
* The right side is the normalized activity name used for aggregation and statistics
* Multiple tokens can map to the same activity node for unified analytics

---

## Multi-Level Activity Example

A more complex hierarchical example:

```toml
parent = "study"

[aliases]
"cs" = "computer-science"
"computer" = "computer-science"
"math" = "math"
"mathematics" = "math"

[aliases.computer-science]
"algorithms" = "algorithm"
"algo" = "algorithm"
"architecture" = "computer-architecture"
"computer architecture" = "computer-architecture"
"networks" = "computer-network"
"computer networks" = "computer-network"

[aliases.math]
"calculus" = "calculus"
"linear algebra" = "linear-algebra"
"linalg" = "linear-algebra"

[aliases.math.calculus]
"multiple integrals" = "multiple-integral"
"double integrals" = "multiple-integral"
```

```toml
parent = "fitness"

[aliases]
"strength training" = "strength-training"
"strength" = "strength-training"
"cardio" = "cardio"
"aerobic" = "cardio"
```

These configurations expand into canonical activity paths such as:

* `algorithms / algo → study_computer-science_algorithm`
* `architecture → study_computer-science_computer-architecture`
* `networks → study_computer-science_computer-network`
* `linear algebra → study_math_linear-algebra`
* `multiple integrals → study_math_calculus_multiple-integral`
* `strength training → fitness_strength-training`
* `cardio → fitness_cardio`

These paths not only convert user tokens into canonical names but also define their position in the hierarchical activity tree.

For example, `multiple integrals → study_math_calculus_multiple-integral` represents time spent on exercises in the calculus subdomain under mathematics. When time is recorded at this leaf node, it is automatically aggregated into all parent nodes:

* `study_math_calculus`
* `study_math`
* `study`

Similarly, `strength training → fitness_strength-training` is aggregated into:

* `fitness_strength-training`
* `fitness`

This allows the same dataset to support both fine-grained analysis and high-level summaries.

From a statistical perspective, this mapping can be understood as a weighted tree:

* Nodes represent canonical activity paths
* Weights represent accumulated durations from leaf nodes
* User input is flexible and free-form, but analysis always operates on normalized paths
* Child node time is recursively aggregated into parent nodes, enabling multi-level inspection of the same dataset

It is important to note that this is a **statistical semantic model**, not the full ingestion pipeline description. The system first parses logs or Android input into normalized activity records and persists them, and only then projects them into hierarchical views during querying or reporting.

The alias resolution rules are:

* `parent` defines the top-level category
* `[aliases.xxx.yyy]` defines intermediate hierarchy levels
* The right-hand side defines leaf-level canonical names
* Final canonical paths are joined using `_`, e.g. `study_math_calculus_multiple-integral`




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
