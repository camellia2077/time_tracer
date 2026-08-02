# User Activity Hierarchy

This directory is the repository-side documentation entry for user-owned
activity hierarchy files.

Activity hierarchy TOML files in the runtime are either:

- imported by the user;
- created or updated by the Android runtime when an unknown activity is
  recorded; or
- edited through configuration management features.

This directory intentionally contains no hierarchy TOML files. Its contents
are excluded from Android and Windows CLI build outputs. Runtime-owned
hierarchy files live in the application's private user-data directory.

For representative TOML examples, see `test/data/activity_hierarchy`.
