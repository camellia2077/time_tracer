import argparse
import os
import sys

BASE_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
SRC_PATH = os.path.join(BASE_DIRECTORY, "src")
sys.path.insert(0, SRC_PATH)

from main import run_tree_generator  # type: ignore


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Tree graph generator.")
    subparsers = parser.add_subparsers(dest="command")

    tree_parser = subparsers.add_parser("tree", help="Generate tree graph from SQLite.")
    tree_parser.add_argument("project_path", nargs="?", default=None, help="Filter tree by project path.")
    tree_parser.add_argument("-l", "--level", type=int, default=None, help="Max depth level.")
    tree_parser.add_argument("--database", help="Override SQLite database path.")
    tree_parser.add_argument("--output-directory", help="Override output directory for images.")
    tree_parser.add_argument("--root-path", help="Override root path filter from config.")
    tree_parser.add_argument("--max-depth", type=int, help="Override max depth from config.")
    tree_parser.add_argument("--output-name", help="Override output file base name.")
    tree_parser.add_argument("--title", help="Override chart title.")
    tree_parser.add_argument("--output-format", choices=["png", "svg", "pdf"],
                             help="Output format for images.")
    tree_parser.add_argument(
        "--output-formats",
        nargs="+",
        help="Output formats (e.g. --output-formats svg png pdf). Comma-separated also supported.",
    )
    tree_parser.add_argument("--output-html", dest="output_html", action="store_true",
                             help="Generate interactive HTML output.")
    tree_parser.add_argument("--no-output-html", dest="output_html", action="store_false",
                             help="Disable interactive HTML output.")
    tree_parser.add_argument("--output-json", dest="output_json", action="store_true",
                             help="Generate JSON data output.")
    tree_parser.add_argument("--no-output-json", dest="output_json", action="store_false",
                             help="Disable JSON data output.")
    tree_parser.add_argument("--output-images", dest="output_images", action="store_true",
                             help="Generate image outputs.")
    tree_parser.add_argument("--no-output-images", dest="output_images", action="store_false",
                             help="Disable image outputs.")
    tree_parser.add_argument("--config-dir", help="Override config directory path.")
    tree_parser.add_argument("--split-roots", dest="split_roots", action="store_true",
                             help="Generate one image per root node.")
    tree_parser.add_argument("--no-split-roots", dest="split_roots", action="store_false",
                             help="Generate a single image for all roots.")
    tree_parser.add_argument("--layout", choices=["horizontal", "vertical"],
                             help="Layout orientation (default: horizontal).")
    tree_parser.add_argument("--horizontal", dest="layout", action="store_const", const="horizontal",
                             help="Force horizontal layout.")
    tree_parser.add_argument("--vertical", dest="layout", action="store_const", const="vertical",
                             help="Force vertical layout.")
    tree_parser.set_defaults(
        split_roots=None,
        output_html=None,
        output_json=None,
        output_images=None,
    )

    return parser


def _resolve_runtime_overrides(args: argparse.Namespace) -> tuple[str | None, int | None, dict, dict]:
    root_path = getattr(args, "project_path", None)
    max_depth = getattr(args, "level", None)
    if getattr(args, "root_path", None):
        root_path = args.root_path
    if getattr(args, "max_depth", None) is not None:
        max_depth = args.max_depth

    path_overrides = {}
    if getattr(args, "database", None):
        path_overrides["database"] = args.database
    if getattr(args, "output_directory", None) is not None:
        path_overrides["output_directory"] = args.output_directory

    setting_overrides = {}
    if getattr(args, "output_name", None) is not None:
        setting_overrides["output_name"] = args.output_name
    if getattr(args, "title", None) is not None:
        setting_overrides["title"] = args.title
    if getattr(args, "split_roots", None) is not None:
        setting_overrides["split_roots"] = args.split_roots
    if getattr(args, "layout", None) is not None:
        setting_overrides["layout"] = args.layout
    if getattr(args, "output_format", None) is not None:
        setting_overrides["output_format"] = args.output_format
    if getattr(args, "output_formats", None) is not None:
        formats: list[str] = []
        if isinstance(args.output_formats, list):
            for item in args.output_formats:
                parts = [part.strip().lower() for part in str(item).split(",") if part.strip()]
                formats.extend(parts)
        else:
            formats = [item.strip().lower() for item in str(args.output_formats).split(",") if item.strip()]
        setting_overrides["output_formats"] = formats
    if getattr(args, "output_html", None) is not None:
        setting_overrides["output_html"] = args.output_html
    if getattr(args, "output_json", None) is not None:
        setting_overrides["output_json"] = args.output_json
    if getattr(args, "output_images", None) is not None:
        setting_overrides["output_images"] = args.output_images

    return root_path, max_depth, path_overrides, setting_overrides


if __name__ == "__main__":
    parser = _build_parser()
    args = parser.parse_args()

    if args.command and args.command != "tree":
        parser.error(f"Unknown command: {args.command}")

    root_path, max_depth, path_overrides, setting_overrides = _resolve_runtime_overrides(args)

    run_tree_generator(
        BASE_DIRECTORY,
        root_path=root_path,
        max_depth=max_depth,
        path_overrides=path_overrides or None,
        setting_overrides=setting_overrides or None,
        config_dir=getattr(args, "config_dir", None),
    )
