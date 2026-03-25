from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.cmd_quality.format import FormatCommand
from tools.toolchain.core.context import Context


class TestFormatCommand(TestCase):
    def test_collect_formattable_files_skips_build_and_out_directories(self):
        with TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            libs_dir = repo_root / "libs" / "tracer_core"
            (libs_dir / "src").mkdir(parents=True)
            (libs_dir / "build_codex").mkdir(parents=True)
            (libs_dir / "out").mkdir(parents=True)

            keep_cpp = libs_dir / "src" / "kept.cpp"
            skip_build_cpp = libs_dir / "build_codex" / "generated.cpp"
            skip_out_hpp = libs_dir / "out" / "generated.hpp"

            keep_cpp.write_text("int main(){return 0;}\n", encoding="utf-8")
            skip_build_cpp.write_text("int ignored(){return 0;}\n", encoding="utf-8")
            skip_out_hpp.write_text("void ignored();\n", encoding="utf-8")

            command = FormatCommand(Context(repo_root))
            files = command._collect_formattable_files(["libs"])

            self.assertEqual(files, [keep_cpp.resolve()])
