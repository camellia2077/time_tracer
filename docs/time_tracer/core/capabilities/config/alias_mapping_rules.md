# Alias Mapping Rules

## Purpose

This document defines the business rules for converter alias mapping under
`assets/tracer_core/config/converter/`.

It answers:

1. what an alias key means
2. what a canonical activity path means
3. which duplicate patterns are allowed or rejected
4. what alias mapping is responsible for, and what it is not responsible for

## Scope

Alias mapping only normalizes user-authored activity-name tokens.

Its responsibility is limited to resolving an alias key into a canonical
activity path.

This layer does not define or carry:

1. time points
2. start/end times
3. durations
4. how many time ranges may reference the same activity
5. how activity records are later inserted into or queried from the database

Those concerns belong to later conversion, persistence, and query stages.

## Terminology

### Alias key

The left-hand value authored by the user in TXT input.

Examples:

```toml
"zh" = "recreation_online-platforms_zhihu"
"zhihu" = "recreation_online-platforms_zhihu"
```

Here, `zh` and `zhihu` are alias keys.

### Canonical activity path

The normalized right-hand value consumed by downstream conversion,
persistence, and query flows.

Examples:

1. `meal_dining`
2. `recreation_online-platforms_zhihu`
3. `study_math_calculus`

## Rules

### 1. Every alias key must be globally unambiguous

A given alias key must always resolve to exactly one canonical activity path.

This is required so that TXT parsing can deterministically map an authored
activity token to one and only one canonical result.

Allowed:

```toml
"zh" = "recreation_online-platforms_zhihu"
"zhihu" = "recreation_online-platforms_zhihu"
```

Rejected:

```toml
"zhihu" = "recreation_online-platforms_zhihu"
"zhihu" = "game_overwatch"
```

### 2. Canonical activity paths do not need to be unique

Different alias keys may resolve to the same canonical activity path.

This is valid because multiple user-authored tokens may intentionally mean the
same activity.

Allowed:

```toml
"zh" = "recreation_online-platforms_zhihu"
"zhihu" = "recreation_online-platforms_zhihu"
```

### 3. Duplicate alias keys are always rejected

Duplicate alias keys are rejected strictly, even if the right-hand value is
identical.

Example of rejected redundant configuration:

```toml
"zhihu" = "recreation_online-platforms_zhihu"
"zhihu" = "recreation_online-platforms_zhihu"
```

This rule exists because repeated declarations are treated as accidental
redundancy rather than useful configuration.

## TOML Shape

## Index file

`alias_mapping.toml` is an index file, not the runtime mapping body.

Example:

```toml
includes = [
  "aliases/meal.toml",
  "aliases/recreation.toml",
]
```

## Child files

Each child file owns one top-level parent and contains an `aliases` table.

Example:

```toml
parent = "recreation"

[aliases.online-platforms]
"zh" = "zhihu"
"zhihu" = "zhihu"
```

This expands to:

```toml
"zh" = "recreation_online-platforms_zhihu"
"zhihu" = "recreation_online-platforms_zhihu"
```

## How To Read A Child File

A child file should be read in four layers:

1. `parent`
   - the top-level canonical path segment
2. table path under `aliases.*`
   - the middle grouping segments under that parent
3. left-hand key
   - the user-authored alias key
4. right-hand value
   - the canonical leaf segment

Example:

```toml
parent = "study"

[aliases.math.calculus]
"不定积分" = "indefinite-integral"
```

This should be read as:

1. top-level parent: `study`
2. middle group path: `math.calculus`
3. alias key: `不定积分`
4. canonical leaf: `indefinite-integral`

The final canonical activity path is:

`study_math_calculus_indefinite-integral`

## Ordering Semantics

Top-level ownership has boundaries, but ordering inside a boundary does not
carry semantic meaning.

### 1. Top-level parents must be split by child file

Each top-level parent belongs to its own child file.

Examples:

1. `meal` rules belong in `aliases/meal.toml`
2. `recreation` rules belong in `aliases/recreation.toml`
3. `study` rules belong in `aliases/study.toml`

This boundary is part of the config organization model and should not be mixed
freely across unrelated parent files.

### 2. Alias entry order inside the same child file is non-semantic

Within the same child file, and within the same alias group such as
`[aliases.online-platforms]`, alias entries may appear in any order.

Their written order affects readability only. It does not change how they are
expanded or resolved.

These two forms are semantically equivalent:

```toml
parent = "recreation"

[aliases.online-platforms]
"zhihu" = "zhihu"
"知乎" = "zhihu"
"douyin" = "douyin"
"抖音" = "douyin"
"bilibili" = "bilibili"
"哔哩哔哩" = "bilibili"
"weibo" = "weibo"
```

```toml
parent = "recreation"

[aliases.online-platforms]
"weibo" = "weibo"
"抖音" = "douyin"
"zhihu" = "zhihu"
"哔哩哔哩" = "bilibili"
"知乎" = "zhihu"
"bilibili" = "bilibili"
"douyin" = "douyin"
```

### 3. Reordering is allowed, duplicate alias keys are not

Ordering freedom does not weaken the uniqueness rules.

This is still rejected:

```toml
parent = "recreation"

[aliases.online-platforms]
"weibo" = "weibo"
"zhihu" = "zhihu"
"weibo" = "weibo"
```

The problem here is not ordering. The problem is that the alias key `weibo`
was declared twice.

## Expansion Rule

String leaves under `aliases` expand as:

`parent + "_" + nested_table_segments + "_" + leaf_value`

Root-level `aliases` leaves omit the middle group portion.

Examples:

1. `parent = "meal"` and `"饭" = "dining"` -> `meal_dining`
2. `parent = "recreation"` and `[aliases.game] "overwatch" = "overwatch"`
   -> `recreation_game_overwatch`

## TOML-Safe Path Segments

Alias child files are stored as TOML table paths such as:

```toml
[aliases.study.math]
```

Because of that encoding, canonical path segments must also be TOML-safe when
they become table-path segments.

### Rule

Unquoted TOML table-path segments must not contain spaces.

So this is invalid:

```toml
[aliases.computer.data structure]
```

This is valid:

```toml
[aliases.computer.data-structure]
```

### Practical guidance

If an old flat canonical path contains a segment with spaces, rewrite that
segment into a TOML-safe form before converting it into child-file table
headers.

Recommended form:

1. use `-` instead of spaces

Example:

1. `study_computer_data structure_stack`
2. rewrite to `study_computer_data-structure_stack`

This rule exists because of TOML syntax, not because alias mapping itself has
special timing, persistence, or query semantics.

## Downstream Relationship

Alias mapping only decides the canonical activity path.

After that:

1. conversion logic derives time ranges and durations from neighboring event
   timestamps
2. persistence logic inserts canonical paths into project and record storage
3. query/reporting logic traverses canonical paths that are already resolved

So alias mapping must be deterministic, but it does not itself define timing
or record multiplicity semantics.

## Related Docs

1. [overview.md](./overview.md)
2. [../validation/txt_logic.md](../validation/txt_logic.md)
3. [../../ingest/txt_to_db_business_logic.md](../../ingest/txt_to_db_business_logic.md)
