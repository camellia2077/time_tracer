from pathlib import PurePosixPath

_DOC_EXTENSIONS = {".md", ".rst", ".txt"}
_SOURCE_EXTENSIONS = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".ipp",
    ".inl",
    ".java",
    ".kt",
    ".kts",
}
_TEST_FILE_SUFFIXES = {
    "_test.c",
    "_test.cc",
    "_test.cpp",
    "_test.cxx",
    "_test.h",
    "_test.hh",
    "_test.hpp",
    "_test.hxx",
}


def is_documentation_file(path: str) -> bool:
    lower = path.lower()
    suffix = PurePosixPath(lower).suffix
    if lower.startswith(("docs/", "temp/", ".agent/")):
        return True
    return suffix in _DOC_EXTENSIONS


def is_cmake_related(path: str) -> bool:
    lower = path.lower()
    file_name = PurePosixPath(lower).name
    if file_name == "cmakelists.txt":
        return True
    if lower.endswith(".cmake"):
        return True
    if lower == "scripts/toolchain/config.toml":
        return True
    if lower.startswith("cmake/") or "/cmake/" in lower:
        return True
    return False


def is_script_file(path: str) -> bool:
    return path.lower().startswith("scripts/")


def is_source_file(path: str) -> bool:
    lower = path.lower()
    suffix = PurePosixPath(lower).suffix
    return suffix in _SOURCE_EXTENSIONS


def is_test_related(path: str) -> bool:
    lower = path.lower()
    posix_path = PurePosixPath(lower)
    if lower.startswith("test/"):
        return True
    if "/test/" in lower or "/tests/" in lower:
        return True
    suffix = posix_path.suffix
    if suffix in _SOURCE_EXTENSIONS:
        for marker in _TEST_FILE_SUFFIXES:
            if lower.endswith(marker):
                return True
    return False
