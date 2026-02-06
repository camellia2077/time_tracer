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
        # Define standard paths
        self.build_tidy_dir = project_dir / "build_tidy"
        self.tasks_dir = self.build_tidy_dir / "tasks"
        self.build_log_path = self.build_tidy_dir / "build.log"

    def run_analyze(self):
        """Analyze Clang-Tidy logs and generate a summary report."""
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
        """Split the main build.log into individual task files."""
        if not self.build_log_path.exists():
            print(f"Error: Build log not found at {self.build_log_path}")
            return

        print(f"Splitting logs from {self.build_log_path} to {self.tasks_dir}...")
        try:
            count = task_splitter.split_tidy_logs(
                self.build_log_path.read_text(encoding="utf-8", errors="replace").splitlines(),
                self.tasks_dir
            )
            print(f"Split complete. {count} task log files created.")
        except Exception as e:
            print(f"Error splitting logs: {e}")

    def run_summary(self, task_ids):
        """Generate a summary for specific task IDs."""
        print(f"Generating batch summary for tasks: {task_ids}")
        summary = log_analyzer.analyze_task_batch(self.tasks_dir, task_ids)
        log_analyzer.print_batch_summary(summary)

    def run_clean(self, task_ids):
        """Cleanup task logs."""
        count = task_manager.cleanup_task_logs(self.tasks_dir, task_ids)
        print(f"Cleanup finished. Total deleted: {count}")

    def list_tasks(self):
        """List available tasks."""
        task_manager.list_tasks(self.tasks_dir)

    def run_fix(self):
        """Trigger the automated fix process via the build system."""
        print("Orchestrating Clang-Tidy Fix...")
        # Call build.py recursively to run the fix build
        build_script = self.script_dir / "build.py"
        # IMPORTANT: Ensure --no-pch is passed to avoid PCH errors during fix
        cmd = [
            sys.executable, str(build_script),
            "--build-dir", "build_tidy_fix",
            "--tidy", "--fix", "--no-pch", "--no-opt", "--no-lto"
        ]
        
        try:
            subprocess.run(cmd, check=True)
            print("Clang-Tidy fix process triggered.")
        except subprocess.CalledProcessError as e:
            print(f"Fix process failed: {e}")
            # We don't exit here, just let the caller handle or continue
            raise e

    def run_auto(self, extra_args):
        """Execute the complete analyze-and-fix workflow."""
        # Step 1: Run Clang-Tidy Analysis Build
        print("=" * 60)
        print("Step 1/4: Running Clang-Tidy Static Analysis...")
        print("=" * 60)
        
        build_script = self.script_dir / "build.py"
        cmd = [
            sys.executable, str(build_script),
            "--build-dir", "build_tidy",
            "--no-pch",
            "--tidy",
            "--analyze-tidy",
            "--split-tasks"
        ] + extra_args
        
        try:
            subprocess.run(cmd, check=False) # Allow warnings/errors
            print("✓ Clang-Tidy Analysis Complete")
        except Exception as e:
             print(f"⚠ Analysis process issue: {e}")

        # Step 2: Split Logs
        print("\n" + "=" * 60)
        print("Step 2/4: Splitting Task Logs...")
        print("=" * 60)
        if self.build_log_path.exists():
            self.run_split()
            print("✓ Task logs split")
        else:
            print(f"✗ Build log not found: {self.build_log_path}")
            return

        # Step 3: Generate Report
        print("\n" + "=" * 60)
        print("Step 3/4: Generating Analysis Report...")
        print("=" * 60)
        self.run_analyze()
        
        summary_file = self.tasks_dir / "tasks_summary.md"
        if summary_file.exists():
             print(f"✓ Report generated: {summary_file}")
             # glob for tasks count
             task_count = len(list(self.tasks_dir.glob("task_*.log")))
             print(f"✓ Found {task_count} tasks to fix")
        else:
             print("⚠ No report generated (possibly no issues found)")

        # Step 4: Auto Fix
        print("\n" + "=" * 60)
        print("Step 4/4: Automated Fix...")
        print("=" * 60)
        
        try:
            self.run_fix()
            print("✓ Auto-fix completed")
            print("\nSuggested Next Steps:")
            print("  1. Check code changes (git diff)")
            print("  2. Re-run verification: sh/refactor/build_tidy.sh")
            print("  3. Run regression tests")
        except Exception as e:
            print(f"⚠ Auto-fix failed: {e}")

        print("\n" + "=" * 60)
        print("✓ Static Analysis Workflow Complete")
        print("=" * 60)
