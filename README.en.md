[English Version](README.en.md) | [中文版本](README.md)

# time tracer (TimeMaster)

**time tracer** - A personal time tracking and analysis system built with C++23.

A powerful personal time management toolset designed using **Clean Architecture** principles to provide maximum efficiency, robust data storage, and multi-dimensional analysis.

### Design Principles (Brief)

1. **Users own their data**: records are stored in readable text, so users can keep, back up, and migrate data without being locked to one app.  
2. **Fast data editing**: users can directly edit text logs (rename activities, add remarks), then sync to database and reports.  
3. **One input format across platforms**: CLI, Android, and other clients use the same text input format to minimize switching cost.  

### Core Components

* **`time_tracker_cli` (C++23)**: The core command-line application. It uses a pipeline pattern to process raw text logs and offers efficient SQLite-based queries and multi-format report exports (Markdown, LaTeX, Typst).
* **`graph_generator` (Python)**: A data visualization tool that reads the database and generates dynamic charts like timelines and heatmaps.
* **`log_generator` (C++)**: A helper utility for generating standardized test logs.

---

## 🚀 Quick Start

### 1. Dependencies

* **C++ Components (`time_tracker_cli`)**:
    * **MSYS2 UCRT64** (Recommended for Windows)
    * **CMake** >= 3.25 (C++23 support)
    * **Compiler**: Clang 16+ or GCC 13+
    * **Libraries**: SQLite3, nlohmann/json, toml++
* **Python Component (`graph_generator`)**:
    * **Python** >= 3.8, Matplotlib

### 2. Build Guide

We provide automated build scripts that compile the core application and all plugin drivers in one go.

➡️ **For detailed steps, see: [Build Guide](docs/time_tracer/guides/build_guide.md)**

### 3. Usage Examples

**Example 1: Automated Ingestion Pipeline (Blink)**
(Validate, Convert, Link, and Persist in one step)

```bash
# Ingest all raw logs from the target_logs directory
time_tracker_cli blink -a "path/to/target_logs"
```

**Example 2: Query Data Records**

```bash
# List all days recorded in 2026
time_tracker_cli query data days --year 2026
```

**Example 3: Export Formatted Reports**

```bash
# Export the weekly report for 2026-W05 as Markdown
time_tracker_cli export week 2026-W05 -f md
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
* **Gemini 2.5 Pro**
* **Gemini 3 Flash**
* **Gemini 3 Pro**
* **Claude 4.5 opus**
* **GPT-5.2-codex**

---

## License & Open Source Libraries

This repository's own source code is licensed under **MIT** (see `LICENSE`).
Third-party dependencies remain under their respective licenses.

### Core and Tools

* **[SQLite](https://www.sqlite.org/)**: Embedded database (Public Domain).
* **[nlohmann/json](https://github.com/nlohmann/json)**: JSON parsing (MIT).
* **[tomlplusplus](https://github.com/marzer/tomlplusplus)**: TOML configuration (MIT).
* **[Matplotlib](https://matplotlib.org/)**: Plotting engine (BSD-style license).

### Android App (`apps/tracer_android`)

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
* `apps/tracer_android/gradle/libs.versions.toml`
