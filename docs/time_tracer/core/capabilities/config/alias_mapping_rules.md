# Alias Mapping Rules

## Purpose

This document defines the business rules for converter alias mapping under
The canonical test source is `test/data/activity_hierarchy/`; the distribution
seed is `assets/tracer_core/defaults/activity_hierarchy/`. Runtime consumers
use the assembled `config/activity_hierarchy/` directory.

It answers:

1. what a canonical leaf key means
2. what an alias array means
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

### Canonical leaf key

The left-hand key in a normal alias entry. It is the canonical leaf segment;
the parent and nested table path are used to build the full canonical activity
path.

Example:

```toml
parent = "recreation"

[canonical.online-platforms]
"zhihu" = ["zhihu", "知乎"]
```

Here, `zhihu` is the canonical leaf key and `zhihu` plus `知乎` are aliases.

### Alias array

The right-hand value of a normal alias entry. It must be a non-empty array of
non-empty strings. Each string is a user-authored token that resolves to the
canonical path represented by the left-hand key.

### Canonical activity path

The normalized right-hand value consumed by downstream conversion,
persistence, and query flows.

Examples:

1. `meal_dining`
2. `recreation_online-platforms_zhihu`
3. `study_math_calculus`

## Rules

### 1. Every alias must be globally unambiguous

A given alias must always resolve to exactly one canonical activity path.

This is required so that TXT parsing can deterministically map an authored
activity token to one and only one canonical result.

Allowed:

```toml
[canonical.online-platforms]
"zhihu" = ["zhihu", "知乎"]
```

Rejected:

```toml
[canonical.online-platforms]
"zhihu" = ["zhihu"]

[canonical.game]
"overwatch" = ["zhihu"]
```

### 2. Canonical activity paths do not need to be unique

Different alias keys may resolve to the same canonical activity path.

This is valid because multiple user-authored tokens may intentionally mean the
same activity.

Allowed:

```toml
"zhihu" = ["zh", "zhihu"]
```

### 3. Duplicate aliases are always rejected

Duplicate aliases are rejected strictly, even if they appear under the same
canonical key or resolve to the same canonical path.

Example of rejected redundant configuration:

```toml
"zhihu" = ["zhihu", "zhihu"]
```

This rule exists because repeated declarations are treated as accidental
redundancy rather than useful configuration.

### 4. Only the new canonical-keyed shape is accepted

Normal entries must use a canonical leaf key and a non-empty string array:

```toml
"rest" = ["rest", "休息", "r"]
```

The legacy alias-to-canonical scalar shape is invalid and is not supported:

```toml
"休息" = "rest"
```

## TOML Shape

## Child files

Each child file owns one top-level parent and contains a `canonical` table.

Example:

```toml
parent = "recreation"

[canonical.online-platforms]
"zhihu" = ["zh", "zhihu"]
```

This expands to:

```text
"zh" -> "recreation_online-platforms_zhihu"
"zhihu" -> "recreation_online-platforms_zhihu"
```

## How To Read A Child File

A child file should be read in four layers:

1. `parent`
   - the top-level canonical path segment
2. table path under `canonical.*`
   - the middle grouping segments under that parent
3. left-hand key
   - the canonical leaf segment
4. right-hand array
   - the user-authored aliases

Example:

```toml
parent = "study"

[canonical.math.calculus]
"indefinite-integral" = ["不定积分"]
```

This should be read as:

1. top-level parent: `study`
2. middle group path: `math.calculus`
3. canonical leaf: `indefinite-integral`
4. aliases: `不定积分`

The final canonical activity path is:

`study_math_calculus_indefinite-integral`

## Ordering Semantics

Top-level ownership has boundaries, but ordering inside a boundary does not
carry semantic meaning.

### 1. Top-level parents must be split by child file

Each top-level parent belongs to its own child file.

Examples:

1. `meal` rules belong in `activity_hierarchy/meal.toml`
2. `recreation` rules belong in `activity_hierarchy/recreation.toml`
3. `study` rules belong in `activity_hierarchy/study.toml`

This boundary is part of the config organization model and should not be mixed
freely across unrelated parent files.

### 2. Alias entry order inside the same child file is non-semantic

Within the same child file, and within the same alias group such as
`[canonical.online-platforms]`, alias entries may appear in any order.

Their written order affects readability only. It does not change how they are
expanded or resolved.

These two forms are semantically equivalent:

```toml
parent = "recreation"

[canonical.online-platforms]
"zhihu" = ["zhihu", "知乎"]
"douyin" = ["douyin", "抖音"]
"bilibili" = ["bilibili", "哔哩哔哩"]
"weibo" = ["weibo"]
```

```toml
parent = "recreation"

[canonical.online-platforms]
"weibo" = ["weibo"]
"douyin" = ["douyin", "抖音"]
"zhihu" = ["zhihu", "知乎"]
"bilibili" = ["bilibili", "哔哩哔哩"]
```

### 3. Reordering is allowed, duplicate alias keys are not

Ordering freedom does not weaken the uniqueness rules.

This is still rejected:

```toml
parent = "recreation"

[canonical.online-platforms]
"weibo" = ["weibo", "weibo"]
```

The problem here is not ordering. The alias `weibo` was declared twice in the
same array.

## Expansion Rule

Each alias string in a normal array under `canonical` expands as:

`parent + "_" + nested_table_segments + "_" + leaf_value`

Root-level `canonical` leaves omit the middle group portion.

Examples:

1. `parent = "meal"` and `"dining" = ["饭"]` -> `meal_dining`
2. `parent = "recreation"` and `[canonical.game] "overwatch" = ["守望先锋"]`
   -> `recreation_game_overwatch`

Group aliases use the same global uniqueness rule and may contain multiple
recordable names for one group:

```toml
[canonical.cardio]
group_aliases = ["有氧训练", "有氧"]
```

Each group alias resolves to the canonical group path. A group alias cannot
also appear in another group or in a normal alias array.

For the category promotion, alias move, TXT replacement, and database rebuild
rules built on this expansion, see [Activity Hierarchy Migration](activity_hierarchy_migration.md).

## TOML-Safe Path Segments

Alias child files are stored as TOML table paths such as:

```toml
[canonical.study.math]
```

Because of that encoding, canonical path segments must also be TOML-safe when
they become table-path segments.

### Rule

Unquoted TOML table-path segments must not contain spaces.

So this is invalid:

```toml
[canonical.computer.data structure]
```

This is valid:

```toml
[canonical.computer.data-structure]
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

## Plaintext Tree Rendering

Core can render one alias child TOML file as plaintext for hierarchy
inspection. The basic mode prints canonical node names only. The alias mode
also prints `aliases` on normal leaves and `group_aliases` on recordable groups.
Aliases remain attributes of their canonical node; they are not rendered as
additional tree nodes. The renderer does not modify TOML, TXT, or the database
and does not provide a JSON configuration-conversion format.

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
