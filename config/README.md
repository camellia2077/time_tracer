# Runtime asset sources

`config/program/` contains immutable program resources used by the
runtime: `config.toml`, charts, bundle metadata, and insights templates.

`config/user_config/` contains the shared source files for user-editable
configuration. These files are separate from the immutable program bundle.

Canonical test activity hierarchy data belongs under
`test/data/activity_hierarchy/` and must not be packaged into Android assets.

The runtime may assemble these sources into its existing Core config root, but
the repository sources must keep program resources and mutable data separate.
