from pathlib import Path
from unittest import TestCase


class TestShellFormatTarget(TestCase):
    def test_shell_format_target_tracks_real_source_families(self):
        repo_root = Path(__file__).resolve().parents[4]
        cmake_text = (
            repo_root
            / "apps"
            / "tracer_core_shell"
            / "cmake"
            / "AddFormatTarget.cmake"
        ).read_text(encoding="utf-8")

        self.assertIn('CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp', cmake_text)
        self.assertIn('CMAKE_CURRENT_SOURCE_DIR}/host/*.cpp', cmake_text)
        self.assertIn('CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp', cmake_text)
        self.assertIn('CMAKE_CURRENT_SOURCE_DIR}/pch*.hpp', cmake_text)
        self.assertIn('CONFIGURE_DEPENDS', cmake_text)
        self.assertNotIn('"src/*.cpp"', cmake_text)
