from __future__ import annotations

import re
from pathlib import Path
from unittest import TestCase


class TestAndroidNativeBridgeSignatureGuard(TestCase):
    def test_native_query_descriptor_matches_kotlin_declaration(self):
        repo_root = Path(__file__).resolve().parents[4]
        kotlin_file = (
            repo_root
            / "apps"
            / "android"
            / "runtime"
            / "src"
            / "main"
            / "java"
            / "com"
            / "example"
            / "tracer"
            / "bridge"
            / "NativeBridge.kt"
        )
        registration_file = (
            repo_root
            / "apps"
            / "tracer_core_shell"
            / "api"
            / "android_jni"
            / "native_bridge_registration.cpp"
        )

        kotlin_source = kotlin_file.read_text(encoding="utf-8")
        registration_source = registration_file.read_text(encoding="utf-8")

        kotlin_types = self._parse_kotlin_native_query_types(kotlin_source)
        descriptor = self._parse_registered_native_query_descriptor(registration_source)
        descriptor_types, return_type = self._parse_jni_descriptor(descriptor)

        self.assertEqual(
            "Ljava/lang/String;",
            return_type,
            "nativeQuery must keep returning java.lang.String for the bridge contract.",
        )
        self.assertEqual(
            kotlin_types,
            descriptor_types,
            "nativeQuery JNI descriptor drifted from NativeBridge.kt; update the "
            "registration string when the Kotlin declaration changes.",
        )

    @staticmethod
    def _parse_kotlin_native_query_types(source: str) -> list[str]:
        match = re.search(
            r"external\s+fun\s+nativeQuery\s*\((?P<body>.*?)\)\s*:\s*String",
            source,
            re.DOTALL,
        )
        if match is None:
            raise AssertionError("Failed to locate NativeBridge.nativeQuery declaration.")

        body = match.group("body")
        types: list[str] = []
        for raw_line in body.splitlines():
            line = raw_line.strip().rstrip(",")
            if not line:
                continue
            param_match = re.match(r"\w+\s*:\s*([A-Za-z0-9?.]+)", line)
            if param_match is None:
                raise AssertionError(f"Unrecognized Kotlin parameter line: {line!r}")
            types.append(TestAndroidNativeBridgeSignatureGuard._map_kotlin_type(param_match.group(1)))
        return types

    @staticmethod
    def _map_kotlin_type(type_name: str) -> str:
        normalized = type_name.rstrip("?")
        if normalized == "Int":
            return "I"
        if normalized == "Boolean":
            return "Z"
        if normalized == "String":
            return "Ljava/lang/String;"
        raise AssertionError(f"Unsupported Kotlin type in nativeQuery declaration: {type_name!r}")

    @staticmethod
    def _parse_registered_native_query_descriptor(source: str) -> str:
        match = re.search(
            r'const_cast<char\*>\("nativeQuery"\),\s*(?://[^\n]*\n\s*)*'
            r'const_cast<char\*>\((?P<body>.*?)\),\s*'
            r"reinterpret_cast<void\*>\(&NativeQuery\)",
            source,
            re.DOTALL,
        )
        if match is None:
            raise AssertionError("Failed to locate nativeQuery JNINativeMethod registration.")

        body = match.group("body")
        parts = re.findall(r'"([^"]*)"', body)
        if not parts:
            raise AssertionError("Failed to read nativeQuery JNI descriptor string segments.")
        return "".join(parts)

    @staticmethod
    def _parse_jni_descriptor(descriptor: str) -> tuple[list[str], str]:
        if not descriptor.startswith("(") or ")" not in descriptor:
            raise AssertionError(f"Invalid JNI descriptor: {descriptor!r}")

        close_index = descriptor.index(")")
        params_blob = descriptor[1:close_index]
        return_type = descriptor[close_index + 1 :]
        params: list[str] = []
        index = 0
        while index < len(params_blob):
            marker = params_blob[index]
            if marker in {"I", "Z"}:
                params.append(marker)
                index += 1
                continue
            if marker == "L":
                end = params_blob.find(";", index)
                if end == -1:
                    raise AssertionError(f"Unterminated object descriptor in {descriptor!r}")
                params.append(params_blob[index : end + 1])
                index = end + 1
                continue
            raise AssertionError(f"Unsupported JNI parameter marker {marker!r} in {descriptor!r}")
        return params, return_type
