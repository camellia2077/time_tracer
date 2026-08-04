# time_tracer clang-tidy workflows

Use the profile-specific document that matches the scope of work:

- `clang_tidy_all_daily.md` / `clang_tidy_all_strict.md`: continue the current queue with bounded agent runs.
- `clang_tidy_by_id_daily.md` / `clang_tidy_by_id_strict.md`: start from one current task, then process its complete source cluster.
- `clang_tidy_all_shared.md` and `clang_tidy_by_id_shared.md`: common contracts referenced by those documents.

There is no `num` workflow and no `tidy-batch` command. The queue is strictly:

```text
scan -> source clusters -> tasks
```

The generated layout is `tasks/clusters/<source_filename>_<hash>/task_<local_id>.*`. A cluster is one source file and all current diagnostics for that file; its task numbers are local and ordered within that directory.

Normal bounded loop:

1. Run `tidy-agent` with a small cluster/task/time budget.
2. If a cluster needs manual work, fix only that source cluster and rerun `tidy-agent` or `tidy-source-step` using a freshly resolved task path.
3. Stop normally when the budget is exhausted. If the runner reports `blocked` or exit code `2`, the agent edits the current source cluster and reruns it; this is not a request for user confirmation.
4. When the current queue is empty, run `tidy-close --dry-run`, then the real `tidy-close` final gate.

`tidy-refresh` always regenerates the full current scan and queue. It is used when source changes make the current queue stale; it is not an incremental batch transition.
