# builder/refactor_orchestrator.py
import sys
import subprocess
from pathlib import Path

from workflow import log_analyzer, task_manager, task_splitter


class RefactorOrchestrator:
    """Orchestrates the analysis, reporting, and fixing workflow."""

    def __init__(self, script_dir: Path, project_dir: Path):
        self.script_dir = script_dir
        self.project_dir = project_dir
        self.build_tidy_dir = project_dir / "build_tidy"
        self.tasks_dir = self.build_tidy_dir / "tasks"
        self.build_log_path = self.build_tidy_dir / "build.log"

    def run_analyze(self):
        print(f"Analyzing Clang-Tidy logs in {self.tasks_dir.resolve()}...")
        if not self.tasks_dir.exists():
            print(f"Error: Tasks directory not found at {self.tasks_dir}")
            return

        summary = log_analyzer.analyze_tasks(self.tasks_dir)
        if not summary:
            print("No tasks found.")
            return

        log_analyzer.save_json_summary(summary, self.tasks_dir / "tasks_summary.json")
        log_analyzer.generate_markdown_summary(summary, self.tasks_dir / "tasks_summary.md", Path.cwd())
        print(f"Analysis complete. {len(summary)} tasks processed.")

    def run_split(self):
        if not self.build_log_path.exists():
            print(f"Error: Build log not found at {self.build_log_path}")
            return

        print(f"Splitting logs from {self.build_log_path} to {self.tasks_dir}...")
        try:
            count = task_splitter.split_tidy_logs(
                self.build_log_path.read_text(encoding="utf-8", errors="replace").splitlines(),
                self.tasks_dir,
            )
            print(f"Split complete. {count} task log files created.")
        except Exception as e:
            print(f"Error splitting logs: {e}")

    def run_summary(self, task_ids):
        print(f"Generating batch summary for tasks: {task_ids}")
        summary = log_analyzer.analyze_task_batch(self.tasks_dir, task_ids)
        log_analyzer.print_batch_summary(summary)

    def run_clean(self, task_ids):
        count = task_manager.cleanup_task_logs(self.tasks_dir, task_ids)
        print(f"Cleanup finished. Total deleted: {count}")

    def list_tasks(self):
        task_manager.list_tasks(self.tasks_dir)

    def run_fix(self):
        print("Orchestrating Clang-Tidy Fix...")
        build_script = self.script_dir / "build.py"
        cmd = [
            sys.executable,
            str(build_script),
            "tidy-fix",
        ]
        try:
            subprocess.run(cmd, check=True)
            print("Clang-Tidy fix process triggered.")
        except subprocess.CalledProcessError as e:
            print(f"Fix process failed: {e}")
            raise e

    def run_auto(self, extra_args):
        print("=" * 60)
        print("Step 1/4: Running Clang-Tidy Static Analysis...")
        print("=" * 60)

        workflow_script = self.script_dir / "workflow.py"
        cmd = [sys.executable, str(workflow_script), "tidy"] + extra_args

        try:
            subprocess.run(cmd, check=False)
            print("✓ Clang-Tidy Analysis Complete")
        except Exception as e:
            print(f"⚠ Analysis process issue: {e}")

        print("\n" + "=" * 60)
        print("Step 2/4: Splitting Task Logs...")
        print("=" * 60)
        if self.build_log_path.exists():
            self.run_split()
            print("✓ Task logs split")
        else:
            print(f"✗ Build log not found: {self.build_log_path}")
            return

        print("\n" + "=" * 60)
        print("Step 3/4: Generating Analysis Report...")
        print("=" * 60)
        self.run_analyze()

        summary_file = self.tasks_dir / "tasks_summary.md"
        if summary_file.exists():
            print(f"✓ Report generated: {summary_file}")
            task_count = len(list(self.tasks_dir.glob("task_*.log")))
            print(f"✓ Found {task_count} tasks to fix")
        else:
            print("⚠ No report generated (possibly no issues found)")

        print("\n" + "=" * 60)
        print("Step 4/4: Automated Fix (Skipping - Handled by Agent)")
        print("=" * 60)

        print("\n" + "=" * 60)
        print("✓ Static Analysis Workflow Complete")
        print("=" * 60)
