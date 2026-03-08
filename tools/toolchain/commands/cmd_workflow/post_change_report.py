from ...services.change_policy import ChangePolicyDecision


def print_decision(decision: ChangePolicyDecision) -> None:
    print("--- post-change policy ---")
    print(f"app: {decision.app_name}")
    print(f"build_dir: {decision.build_dir_name}")
    print(f"changed_files: {len(decision.changed_files)}")

    preview_limit = 12
    for path in decision.changed_files[:preview_limit]:
        print(f"  - {path}")
    if len(decision.changed_files) > preview_limit:
        print(f"  - ... ({len(decision.changed_files) - preview_limit} more)")

    print("reasons:")
    for reason in decision.reasons:
        print(f"  - {reason}")

    print(
        "actions: "
        f"configure={decision.need_configure}, "
        f"build={decision.need_build}, "
        f"test={decision.need_test}"
    )
