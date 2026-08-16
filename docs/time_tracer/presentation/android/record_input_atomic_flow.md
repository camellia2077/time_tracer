# Android Record Input Atomic Flow

## Purpose

Explain the Android `Record Input` business flow from single-line authoring to
official TXT update and database sync.

This page focuses on:

1. what Android validates before record submission
2. what the runtime and core pipeline validate authoritatively
3. how one record becomes a candidate TXT update
4. why the flow is atomic instead of "write TXT first, fix later"

For the target interval-event semantics, see:
1. [interval_event_and_mixed_timeline_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md)

## Scope

This page covers the `Record` tab single-line input flow.

It does not cover:

1. full TXT editor save semantics
2. insights rendering behavior
3. config asset packaging details
4. the future explicit interval-event authoring flow

## Core Idea

Android single-line recording is not a "local append then best-effort sync"
flow.

It is an atomic authoring flow:

`author input -> candidate month TXT -> full core validation -> official TXT replace -> single-month ingest -> success or rollback`

That means:

1. Android may do lightweight authoring checks first
2. the final authority still lives in core atomic record
3. TXT and DB should stay aligned after a successful record action

Important scope note:

1. this page describes the current point-event atomic flow
2. it does not yet describe the future explicit interval-event authoring path
3. when interval authoring is introduced, it should reuse the same
   `candidate TXT -> full validation -> official replace -> ingest or rollback`
   authority model

## Flow Summary

The effective flow is:

1. User types an activity token and optional remark in `Record Input`
2. Android resolves the logical target day (`today` or `yesterday`)
3. Android calls runtime `recordNow(...)`
4. Runtime delegates to core atomic record
5. Core builds a candidate month TXT in memory
6. Core validates the candidate through the ingest pipeline
7. If validation succeeds, core replaces the official TXT and re-imports that month
8. If import fails, core rolls back TXT

## 1. Android Authoring Layer

### 1.1 Authorable Token Set

When the user enters the `Record` or `TXT` tab, Android loads:

`authorable_event_tokens = alias_mapping.keys ∪ wake_keywords`

See:

1. [TracerTabs.kt](/C:/code/time_tracer/apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt)
2. [QueryGateway.kt](/C:/code/time_tracer/apps/android/contract/src/main/java/com/example/tracer/QueryGateway.kt)

Business meaning:

1. ordinary activity tokens are authorable
2. wake tokens are also authorable
3. Android must not hardcode wake strings locally

### 1.2 What Android Uses This Set For

Android uses that set for lightweight authoring-side checks in:

1. Quick Access search candidates
2. Quick Access save validation
3. frequent-activity click validation

See:

1. [RecordTabContent.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt)

Important boundary:

`Record Input` free typing is not finally accepted or rejected by Compose UI
alone. The authoritative acceptance check happens in core atomic record.

### 1.3 Current Haptic Feedback

Current Android `Record` interactions provide haptic feedback in:

1. `Quick Access`: long press to enter drag-reorder triggers one haptic feedback
2. `Record Activity`: triggering the record action provides one haptic feedback

### 1.4 Add Activity Browser

`Add activity` has three sources:

1. `Tree`: select a canonical activity path
2. `Frequent`: select a recently frequent activity
3. `Categories`: maintain canonical paths and aliases in the Add activity
   browser itself

Tree and Frequent selections fill the Record Input activity field. Categories
does not select an activity; it hosts the sole activity-hierarchy editor. A
successful save refreshes the Tree source so the user can select the changed
activity without leaving Record Input.

## 2. ViewModel And Use Case Layer

When the user taps record:

1. `RecordViewModel.recordNow()` is called
2. `RecordUseCases.recordNow(...)` resolves the target logical date
3. `RecordUseCases` also resolves the time-order mode

See:

1. [RecordViewModel.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt)
2. [RecordUseCases.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCases.kt)

Current business rule:

1. `TODAY` uses `STRICT_CALENDAR`
2. `YESTERDAY` uses `LOGICAL_DAY_0600`

This allows late-night logical-day recording to stay consistent with the
Record/TXT shared target-day model.

## 3. Runtime Entry

Android runtime receives:

1. `activityName`
2. `remark`
3. `targetDateIso`
4. optional preferred TXT path
5. selected time-order mode

Then [RuntimeRecordDelegate.kt](/C:/code/time_tracer/apps/android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeRecordDelegate.kt)
calls the native atomic record bridge instead of maintaining a separate local
"append text then sync" implementation.

This is deliberate:

1. one authority for validation
2. one authority for candidate TXT generation
3. one authority for rollback behavior

## 4. Core Atomic Record Authority

The core authority lives in:

1. [pipeline_record_alias_text_support_impl.inc](/C:/code/time_tracer/libs/tracer_core/src/application/pipeline/detail/pipeline_record_alias_text_support_impl.inc)
2. [pipeline_record_atomic_support_impl.inc](/C:/code/time_tracer/libs/tracer_core/src/application/pipeline/detail/pipeline_record_atomic_support_impl.inc)

### 4.1 Activity Name Validation

Core first validates the raw activity token.

Current rule:

1. token is valid if it is an alias child file key, or
2. token is valid if it is in `wake_keywords`

Business meaning:

1. wake classification is config-driven
2. authorable validation is aligned with Android's `authorable_event_tokens`
3. TXT keeps the user-entered raw token text unchanged

The alias-to-canonical mapping still happens later during full TXT parse and
ingest, not during record-time text authoring.

### 4.2 Candidate TXT Construction

Core does not write directly into the official TXT first.

It builds a candidate month TXT in memory:

1. resolve target month and target day marker
2. create the month template if the file does not exist
3. generate current local ISO time `HH:mm:ss`
4. build one raw point-event line:
   compact TXT line `HHMMSS + activity + optional remark`
5. insert that line into the target day block
6. if the day does not exist yet, create the day block

Business meaning:

1. one single-line authoring action is always judged in whole-document context
2. record validation is still month-TXT validation, not isolated line regex validation
3. future interval-event authoring should follow the same whole-document rule,
   even if the user provides explicit start/end time

### 4.3 Record-Time Checks

During candidate construction, core applies record-time checks such as:

1. activity token must be valid
2. target date must be valid
3. preferred TXT path must stay inside input root
4. new event time must be strictly later than the last event time in that day,
   under the selected time-order mode
5. same-day duplicate `HHMMSS + activity` yields a warning

If the day remains incomplete after insertion:

1. generic completeness warning may be added
2. overnight continuation warning may replace it when the first authored event
   is not wake-related

These are warnings, not failures.

## 5. Full Candidate Validation

After candidate TXT is built, core runs the candidate through the normal ingest
validation stages:

1. structure validation
2. conversion
3. cross-month link
4. logic validation

This means a single record action is still checked against the full business
rules of the TXT-first ingest model.

Examples of blocking failures:

1. invalid target date
2. invalid activity token
3. impossible time ordering in the target day
4. wake appearing in a non-first semantic position after candidate insertion
5. any other structure/logic rule violated by the candidate month document

For the target interval-event model, the same validation boundary should also
apply to:

1. explicit interval overlap
2. interval start/end ordering errors
3. mixed point-event and interval-event timeline conflicts

## 6. Official TXT Replace And DB Sync

If candidate validation succeeds:

1. core backs up the official TXT if it already exists
2. core writes the candidate content into the official TXT
3. core runs `single_txt_replace_month` ingest for that month

If ingest succeeds:

1. the action is committed
2. TXT and DB are both updated

If ingest fails:

1. core restores the old TXT from backup
2. the record action fails
3. TXT and DB remain consistent with the pre-action state

This is why the flow is called atomic in practice.

## 7. What A Record Input Action Means Business-Wise

In Android `Record Input`, the user only authors:

1. activity token
2. optional remark

For point-event authoring, the runtime supplies:

1. current ISO time `HH:mm:ss` (converted to compact `HHMMSS` only for TXT)
2. target logical day
3. target month TXT path

So the single-line action means:

1. append one new point event into the logical day
2. re-evaluate the whole month TXT under ingest rules
3. if valid, persist the updated month and sync DB

The line itself is not yet a complete duration fact.

Its final duration semantics still depend on:

1. previous event time in the day
2. cross-day previous context
3. wake semantics
4. generated activities such as `sleep_night`

For interval-event authoring, Android must also supply the absolute
`attribution_date` captured when the interval starts. The save time is not a
replacement for that date. A cross-midnight interval remains one interval and
is written to the day block identified by its fixed start logical day.

For example, an interval started at `2026-07-22 23:00:00` and stopped at
`2026-07-23 07:00:00` is saved as one 8-hour interval under `2026-07-22`.
Editing its start/end time does not silently move it to another day.

### 7.1 Previous Activity End-Time Suggestion

When the user is authoring a completed interval draft, Android may ask
core/runtime for the latest persisted activity end boundary at or before the
selected logical day. The query is read-only and returns either no result or a
semantic `{date, end_time}` boundary.

If a boundary is available, Record Input shows the end time as a suggestion and
offers a one-click action to copy that value into the interval draft's start
time. This is an explicit user action: the query does not change the draft,
does not save a record, and its suggestion is not shown while an active
interval timer is being displayed. The user can still edit the start time
manually before saving. Android only offers the action when the returned
boundary belongs to the selected logical day; an earlier day's `HH:mm:ss` is not
copied into the current day.

### 7.2 Latest Persisted Activity Summary

The `Last` activity summary in Record Input is loaded through the read-only
core/runtime `latest_activity_record` query rather than being treated as a
session-only UI result. Core selects the latest persisted record for the
selected logical day and returns its activity plus start and end boundaries.
For a point/end-only record, the missing start boundary is displayed as `—`.
The summary refreshes when Record Input starts, when the logical day changes,
and after a successful record operation.

## 8. Android Vs Core Responsibility Boundary

### 8.1 Android Responsibilities

Android is responsible for:

1. collecting author input
2. loading authorable token sets from core/runtime
3. lightweight authoring-side validation for Quick Access and frequent activities
4. resolving and pinning the interval attribution date at start time
5. persisting that absolute date with an unfinished interval draft
6. resolving target logical day and time-order mode for point events and
   interval drafts that have no pinned date
7. surfacing success, warning, and failure messages

### 8.2 Core Responsibilities

Core is responsible for:

1. authoritative activity-token validation
2. candidate TXT construction
3. month-document validation
4. official TXT update
5. month re-import into DB
6. rollback on failure

Core receives the already selected absolute target date for an interval. It
continues to own interval validation, cross-midnight duration semantics,
overlap detection, TXT replacement, and DB synchronization.

When interval-event authoring is added, these responsibilities should remain in
core as well; Android should not independently invent a local interval-only
validation or persistence path.

This boundary is intentional:

1. Android stays thin
2. business logic remains centralized
3. TXT-first semantics are preserved across CLI, Android, and future surfaces

## 9. Why This Design Exists

This design avoids several failure modes:

1. Android and core disagreeing on what counts as a valid activity token
2. Android appending text locally that core later rejects
3. official TXT changing while DB stays stale
4. partial success where a record appears in TXT but not in DB
5. duplicated business logic across UI and ingest codepaths

## 10. Related Documents

For adjacent topics, open:

1. [runtime-protocol.md](/C:/code/time_tracer/docs/time_tracer/presentation/android/runtime-protocol.md)
2. [txt_to_db_business_logic.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/txt_to_db_business_logic.md)
3. [record_input_and_day_completeness_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/record_input_and_day_completeness_semantics.md)
4. [day_bucket_and_wake_anchor_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/day_bucket_and_wake_anchor_semantics.md)
5. [interval_event_and_mixed_timeline_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md)
