import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.tidy import analysis_compile_db  # noqa: E402


class TestAnalysisCompileDb(TestCase):
    def test_sanitize_command_text_removes_modmap_arg(self):
        command = (
            r"C:\clang++.exe -DTT_ENABLE_CPP20_MODULES=0 -std=gnu++23 "
            r"@libs\tracer_core\CMakeFiles\tc_app_lib.dir\src\foo.cpp.obj.modmap "
            r"-o libs\tracer_core\CMakeFiles\tc_app_lib.dir\src\foo.cpp.obj -c C:\repo\foo.cpp"
        )
        sanitized = analysis_compile_db.sanitize_command_text(command)
        self.assertNotIn(".obj.modmap", sanitized)
        self.assertIn("-o libs\\tracer_core\\CMakeFiles\\tc_app_lib.dir\\src\\foo.cpp.obj", sanitized)

    def test_ensure_analysis_compile_db_writes_copy(self):
        with TemporaryDirectory() as tmp:
            build_dir = Path(tmp)
            payload = [
                {
                    "directory": str(build_dir),
                    "command": (
                        r"C:\clang++.exe @foo.obj.modmap -o foo.obj -c C:\repo\foo.cpp"
                    ),
                    "file": r"C:\repo\foo.cpp",
                }
            ]
            (build_dir / "compile_commands.json").write_text(
                json.dumps(payload),
                encoding="utf-8",
            )

            out_dir = analysis_compile_db.ensure_analysis_compile_db(build_dir)
            out_path = out_dir / "compile_commands.json"
            self.assertTrue(out_path.exists())
            data = json.loads(out_path.read_text(encoding="utf-8"))
            self.assertNotIn(".obj.modmap", data[0]["command"])

    def test_ensure_analysis_compile_db_strips_modmap_from_arguments(self):
        with TemporaryDirectory() as tmp:
            build_dir = Path(tmp)
            payload = [
                {
                    "directory": str(build_dir),
                    "arguments": [
                        r"C:\clang++.exe",
                        r"@foo.obj.modmap",
                        r"-o",
                        r"foo.obj",
                        r"-c",
                        r"C:\repo\foo.cpp",
                    ],
                    "file": r"C:\repo\foo.cpp",
                }
            ]
            (build_dir / "compile_commands.json").write_text(
                json.dumps(payload),
                encoding="utf-8",
            )

            out_dir = analysis_compile_db.ensure_analysis_compile_db(build_dir)
            out_path = out_dir / "compile_commands.json"
            data = json.loads(out_path.read_text(encoding="utf-8"))
            self.assertNotIn(r"@foo.obj.modmap", data[0]["arguments"])
