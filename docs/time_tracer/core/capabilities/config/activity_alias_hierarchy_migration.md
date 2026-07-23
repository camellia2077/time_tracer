# Activity Alias Hierarchy Migration

## Purpose

This document defines how an activity alias becomes a recordable category, how
an alias moves into a category, and when that structural edit requires TXT and
database migration.

It applies to every client that edits files under
`assets/tracer_core/config/aliases/`.

## Terms

- **Activity alias**: a string leaf such as `"跑步" = "running"`.
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
"跑步" = "running"
```

- `有氧运动` resolves to `exercise_cardio`.
- `跑步` resolves to `exercise_cardio_running`.

## Promote an Alias to a Category

Initial state:

```toml
parent = "exercise"

[aliases]
"有氧运动" = "cardio"
"跑步" = "running"
```

The canonical paths are:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_running`

Promoting `有氧运动` creates a category whose name is the old canonical leaf.
The category automatically retains the original alias as its record name:

```toml
parent = "exercise"

[aliases]
"跑步" = "running"

[aliases.cardio]
group_aliases = ["有氧运动"]
```

The resulting canonical paths are unchanged:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_running`

This operation changes TOML structure only. It does **not** replace TXT tokens
or rebuild the database.

## Move an Alias into a Category

Moving `跑步` into `cardio` produces:

```toml
parent = "exercise"

[aliases.cardio]
group_aliases = ["有氧运动"]
"跑步" = "running"
```

The paths then become:

- `有氧运动` → `exercise_cardio`
- `跑步` → `exercise_cardio_running`

Because `跑步` changes from `exercise_running` to
`exercise_cardio_running`, confirmation must perform this atomic migration:

1. update the alias TOML;
2. replace matching canonical activity tokens in TXT files;
3. construct a candidate database by re-ingesting the updated TXT files;
4. replace the active database only after the candidate succeeds;
5. restore TOML, TXT, and database state if any stage fails.

Only a leaf alias can be moved in the current implementation. A category and
its descendants cannot be moved as one operation.

## Edit Category Record Names

For a category with an existing `group_aliases` member:

- **Rename a record name**: update TOML, replace the old activity token in TXT,
  and rebuild the database. The category canonical path itself does not change.
- **Add a record name**: update TOML only. No historic TXT token needs to be
  changed and no database rebuild is required.

The Config UI exposes record-name editing only for categories that already have
one or more category record names. It exposes adding a record name from the
same category edit menu.

## Related Documents

- [Alias Mapping Rules](alias_mapping_rules.md): alias uniqueness and canonical
  expansion rules.
- [Runtime TXT Day-Block JSON Contract](../../contracts/text/runtime_txt_day_block_json_contract_v1.md):
  exact parsed-token replacement contract used by migration.
