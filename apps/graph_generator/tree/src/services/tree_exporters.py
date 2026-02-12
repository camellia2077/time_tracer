import json
import multiprocessing as mp
import os
from dataclasses import dataclass
from pathlib import Path
from typing import List, Protocol, Tuple

from jinja2 import Environment, FileSystemLoader, select_autoescape
from markupsafe import Markup

from core.config import TreeConfig
from core.models import TreeNode
from rendering.tree_renderer import TreeRenderer


@dataclass(slots=True)
class ExportRequest:
    roots: List[TreeNode]
    title: str
    output_name: str


class TreeExporter(Protocol):
    def export(self, request: ExportRequest) -> None:
        ...


def _node_to_dict(node: TreeNode, prefix: str) -> dict:
    path = f"{prefix}_{node.name}" if prefix else node.name
    return {
        "name": node.name,
        "path": path,
        "duration_seconds": node.duration_seconds,
        "duration_text": _format_duration(node.duration_seconds),
        "duration_days_text": _format_duration_days(node.duration_seconds),
        "children": [_node_to_dict(child, path) for child in node.children],
    }


def _format_duration(seconds: int) -> str:
    if seconds <= 0:
        return "0m"
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    if hours > 0:
        return f"{hours}h {minutes}m"
    if minutes > 0:
        return f"{minutes}m"
    return f"{seconds}s"


def _format_duration_days(seconds: int) -> str:
    if seconds <= 0:
        return "0.00d"
    days = seconds / 86400.0
    return f"{days:.2f}d"


def _render_job(args: Tuple[TreeConfig, str, List[TreeNode], str, str, str]) -> None:
    config, output_dir, roots, title, output_name, output_format = args
    renderer = TreeRenderer(output_dir, config)
    renderer.render(roots, title, output_name, output_format)


class ImageTreeExporter:
    """Exports tree charts as png/svg/pdf images."""

    def __init__(self, config: TreeConfig, output_dir: str):
        self.config = config
        self.output_dir = output_dir

    def export(self, request: ExportRequest) -> None:
        jobs = self._build_jobs(
            roots=request.roots,
            title=request.title,
            output_name=request.output_name,
            split_roots=self.config.settings.split_roots,
            formats=self.config.resolved_output_formats(),
        )
        self._run_parallel(jobs)

    def _build_jobs(
        self,
        roots: List[TreeNode],
        title: str,
        output_name: str,
        split_roots: bool,
        formats: List[str],
    ) -> List[Tuple[TreeConfig, str, List[TreeNode], str, str, str]]:
        jobs: List[Tuple[TreeConfig, str, List[TreeNode], str, str, str]] = []
        if split_roots and len(roots) > 1:
            for root in roots:
                safe_name = self._safe_name(root.name)
                root_output = f"{output_name}_{safe_name}" if output_name else safe_name
                root_title = f"{title} - {root.name}" if title else root.name
                for output_format in formats:
                    jobs.append((self.config, self.output_dir, [root], root_title, root_output, output_format))
            return jobs

        for output_format in formats:
            jobs.append((self.config, self.output_dir, roots, title, output_name, output_format))
        return jobs

    def _run_parallel(self, jobs: List[Tuple[TreeConfig, str, List[TreeNode], str, str, str]]) -> None:
        if not jobs:
            return

        if len(jobs) == 1:
            _render_job(jobs[0])
            return

        worker_count = min(len(jobs), max(1, mp.cpu_count() - 1))
        if worker_count <= 1:
            for job in jobs:
                _render_job(job)
            return

        ctx = mp.get_context("spawn")
        try:
            with ctx.Pool(processes=worker_count) as pool:
                pool.map(_render_job, jobs)
        except (PermissionError, OSError) as exc:
            print(f"Parallel rendering unavailable, falling back to sequential mode: {exc}")
            for job in jobs:
                _render_job(job)

    @staticmethod
    def _safe_name(name: str) -> str:
        safe_chars = []
        for ch in name:
            if ch.isalnum() or ch in ("-", "_"):
                safe_chars.append(ch)
            elif ch.isspace():
                safe_chars.append("_")
            else:
                safe_chars.append("_")
        result = "".join(safe_chars).strip("_")
        return result or "root"


class JsonTreeExporter:
    """Exports tree data to JSON."""

    def __init__(self, output_dir: str):
        self.output_dir = output_dir

    def export(self, request: ExportRequest) -> None:
        os.makedirs(self.output_dir, exist_ok=True)
        json_path = os.path.join(self.output_dir, f"{request.output_name}.json")
        json_text = self._build_json_text(request)
        with open(json_path, "w", encoding="utf-8") as handle:
            handle.write(json_text)

    def _build_json_text(self, request: ExportRequest) -> str:
        data = {
            "title": request.title,
            "roots": [_node_to_dict(root, "") for root in request.roots],
        }
        return json.dumps(data, ensure_ascii=False, indent=2)


class HtmlTreeExporter:
    """Exports interactive HTML (with embedded JSON fallback)."""

    def __init__(self, output_dir: str):
        self.output_dir = output_dir

    def export(self, request: ExportRequest) -> None:
        os.makedirs(self.output_dir, exist_ok=True)
        json_filename = f"{request.output_name}.json"
        html_path = os.path.join(self.output_dir, f"{request.output_name}.html")
        json_text = self._build_json_text(request)
        html = self._build_html(request.title, json_filename, json_text)
        with open(html_path, "w", encoding="utf-8") as handle:
            handle.write(html)

    def _build_json_text(self, request: ExportRequest) -> str:
        data = {
            "title": request.title,
            "roots": [_node_to_dict(root, "") for root in request.roots],
        }
        return json.dumps(data, ensure_ascii=False, indent=2)

    def _build_html(self, title: str, json_filename: str, json_text: str) -> str:
        templates_dir = Path(__file__).resolve().parents[2] / "templates"
        env = Environment(
            loader=FileSystemLoader(str(templates_dir)),
            autoescape=select_autoescape(["html", "xml"]),
        )
        template = env.get_template("tree_view.html")
        return template.render(
            title=title,
            json_filename=json_filename,
            json_text=Markup(json_text),
        )
