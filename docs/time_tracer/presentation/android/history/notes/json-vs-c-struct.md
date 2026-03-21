# Android Note: JSON vs C struct

## Purpose

Archive an older Android-side protocol discussion about JSON payloads versus C struct boundaries.

## When To Open

- Open this only when you need the historical reasoning behind the current JSON-based business payload choice.

## What This Doc Does Not Cover

- Current routing for Android runtime changes
- Full runtime protocol specification
- Current performance measurements

## Historical Context

At the time of the note:

1. Android already used the C ABI boundary (`tracer_core_*`).
2. Business requests and responses still crossed the Kotlin/JNI/runtime boundary as JSON strings.
3. The call chain was:
   - Kotlin/Compose -> JNI
   - JNI -> `tracer_core_*` C ABI
   - C ABI payload/response -> `const char*` JSON

## Historical Conclusion

The recommended choice was to keep JSON as the business protocol rather than switch to C structs.

Why:

1. Lower cross-language integration cost for Kotlin and future hosts.
2. More stable protocol evolution through optional fields and loose schema growth.
3. No clear evidence that JSON encoding was the main performance bottleneck.

## Tradeoff Summary

JSON advantages:

1. Easier debugging and logging.
2. Easier compatibility evolution.
3. Lower multi-host integration cost.

JSON disadvantages:

1. Serialization overhead.
2. Weaker compile-time type guarantees.

C struct advantages:

1. Better theoretical performance.
2. Tighter static type boundaries.

C struct disadvantages:

1. Much stricter ABI stability burden.
2. Higher JNI/Kotlin mapping complexity.
3. Harder protocol evolution.

## Historical Follow-Up Trigger

The note suggested reconsidering C structs only if:

1. Profiling proved JSON encode/decode was the bottleneck.
2. A high-frequency, large-payload runtime hotspot was identified.
3. The team was ready to maintain stricter ABI-versioning and memory-ownership rules.
