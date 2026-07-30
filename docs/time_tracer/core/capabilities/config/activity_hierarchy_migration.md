# Activity Hierarchy Migration

## Purpose

This document defines how an activity alias becomes a recordable category, how
an alias moves into a category, and when that structural edit requires TXT and
database migration.

It applies to every client that edits alias files generated from
`test/data/activity_hierarchy/` or
`assets/tracer_core/defaults/activity_hierarchy/`.

## Terms

- **Canonical key**: the recordable activity identifier on the left side of a
  TOML entry, such as `"running"`.
- **Activity alias**: a string in the canonical key's alias array, such as
  `"跑步"` in `"running" = ["跑步"]`.
- **Activity category**: a nested TOML table such as `[aliases.cardio]`.
- **Category record name**: a member of a category's `group_aliases` array.
  It records directly to that category's canonical path.

`group_aliases` is category metadata, not a normal alias leaf. Alias keys,
including category record names, must remain globally unique.

## Canonical Expansion

A normal alias leaf resolves as:

`parent + "_" + category path + "_" + canonical leaf`

A category record name resolves as:

`parent + "_" + category path`

For example:

```toml
parent = "exercise"

[aliases.cardio]
group_aliases = ["有氧运动"]
"running" = ["跑步"]
```

- `有氧运动` resolves to `exercise_cardio`.
- `跑步` resolves to `exercise_cardio_running`.

## Promote an Alias to a Category

Initial state:

```toml
parent = "exercise"

[aliases]
"cardio" = ["有氧运动"]
"running" = ["跑步"]
```

The canonical paths are:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_running`

Promoting `有氧运动` creates a category whose name is the old canonical leaf.
The category automatically retains the original alias as its record name:

```toml
parent = "exercise"

[aliases]
"running" = ["跑步"]

[aliases.cardio]
group_aliases = ["有氧运动"]
```

The resulting canonical paths are unchanged:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_running`

Core returns an updated TOML with an empty token replacement plan for this
operation. The host still commits the document through
`RuntimeActivityHierarchyMigrationService`, which validates the candidate configuration
and database before activation; an empty plan never authorizes a direct TOML
save.

## Move an Alias into a Category

Moving `跑步` into `cardio` produces:

```toml
parent = "exercise"

[aliases.cardio]
group_aliases = ["有氧运动"]
"running" = ["跑步"]
```

The paths then become:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_cardio_running`

Because `跑步` changes from `exercise_running` to
`exercise_cardio_running`, confirmation must perform this atomic migration:

1. Android/CLI sends the operation and user input to Core;
2. Core returns the updated TOML and canonical/alias replacement plan;
3. `RuntimeActivityHierarchyMigrationService` replaces matching tokens in TXT files;
4. the service constructs a candidate database from the updated TOML and TXT;
5. TOML, TXT, and database are atomically activated only after success, and
   restored if any stage fails.

Leaf and group nodes can be moved between alias TOML documents. A leaf move
includes root-level and nested activities. A group move includes the complete
subtree: the group, its group aliases, nested groups, and leaf activities. Core
receives the complete alias TOML document set, validates global alias
uniqueness, and produces the updated source and destination documents plus a
canonical replacement for every moved node whose path changes; the host must
commit both documents, all TXT changes, and the candidate database in one
atomic migration.

## Edit Category Record Names

For a category with an existing `group_aliases` member:

- **Rename a record name**: Core returns an `alias_replacements` entry; the
  migration service replaces the old token in TXT and rebuilds the database.
  The category canonical path itself does not change.
- **Add a record name**: Core returns an updated TOML with an empty replacement
  plan; it still goes through the migration service so TOML/TXT/database state
  is committed consistently.

The Config UI exposes record-name editing only for categories that already have
one or more category record names. It exposes adding a record name from the
same category edit menu.

## Related Documents

- [Alias Mapping Rules](alias_mapping_rules.md): alias uniqueness and canonical
  expansion rules.
- [Runtime TXT Day-Block JSON Contract](../../contracts/text/runtime_txt_day_block_json_contract_v1.md):
  exact parsed-token replacement contract used by migration.
