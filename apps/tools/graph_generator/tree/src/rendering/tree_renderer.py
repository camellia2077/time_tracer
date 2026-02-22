import os
from typing import Any, Dict, List, Tuple

import matplotlib.pyplot as plt
from matplotlib import colors as mpl_colors

from core.config import TreeConfig
from core.models import TreeNode

class TreeRenderer:
    """Renders a project tree using matplotlib."""
    def __init__(self, output_dir: str, config: TreeConfig):
        self.output_dir = output_dir
        self.config = config
        self._setup_matplotlib()
        self._load_style()

    def _setup_matplotlib(self) -> None:
        plt.rcParams["font.family"] = "sans-serif"
        plt.rcParams["font.sans-serif"] = ["SimHei", "Microsoft YaHei"]
        plt.rcParams["axes.unicode_minus"] = False

    def _load_style(self) -> None:
        style = self.config.style
        settings = self.config.settings

        self.figure_width = self._coerce_float(style.figure_width, 16.0)
        self.figure_height = self._coerce_float(style.figure_height, 10.0)
        self.dpi = int(self._coerce_float(style.dpi, 300.0))

        self.background_color = str(style.background_color)
        self.node_size = self._coerce_float(style.node_size, 280.0)
        self.root_color = str(style.root_color)
        self.edge_color = str(style.edge_color)

        self.label_font_size = int(self._coerce_float(style.label_font_size, 10.0))
        self.label_offset_y = self._coerce_float(style.label_offset_y, 0.25)

        self.horizontal_spacing = self._coerce_float(style.horizontal_spacing, 1.4)
        self.vertical_spacing = self._coerce_float(style.vertical_spacing, 1.2)
        self.layout = str(settings.layout or "horizontal").strip().lower()
        self.root_node_scale = self._coerce_float(style.root_node_scale, 1.5)
        self.min_node_scale = self._coerce_float(style.min_node_scale, 0.6)
        self.label_root_scale = self._coerce_float(style.label_root_scale, 1.2)
        self.label_min_scale = self._coerce_float(style.label_min_scale, 0.85)
        self._apply_color_scheme()

    def render(self, roots: List[TreeNode], title: str, output_name: str, output_format: str) -> None:
        if not roots:
            print("No nodes to render.")
            return

        positions, edges, name_map, depth_map, max_depth = self._layout(roots)
        fig, ax = plt.subplots(figsize=(self.figure_width, self.figure_height))
        fig.patch.set_facecolor(self.background_color)
        ax.set_facecolor(self.background_color)

        for parent_id, child_id in edges:
            x1, y1 = positions[parent_id]
            x2, y2 = positions[child_id]
            depth = depth_map.get(parent_id, 0)
            line_width = self._edge_width_for_depth(depth, max_depth)
            ax.plot([x1, x2], [y1, y2], color=self.edge_color, linewidth=line_width, zorder=1)

        xs, ys, sizes, colors = [], [], [], []
        for node_id, (x, y) in positions.items():
            depth = depth_map.get(node_id, 0)
            size = self._node_size_for_depth(depth, max_depth)
            color = self._color_for_depth(depth, max_depth)
            xs.append(x)
            ys.append(y)
            sizes.append(size)
            colors.append(color)

        if xs:
            ax.scatter(xs, ys, s=sizes, c=colors, edgecolors="black", zorder=2)

        label_offset = self.label_offset_y * self.vertical_spacing
        for node_id, (x, y) in positions.items():
            depth = depth_map.get(node_id, 0)
            font_size = self._label_size_for_depth(depth, max_depth)
            ax.text(x, y + label_offset, name_map[node_id], ha="center", va="bottom",
                    fontsize=font_size)

        self._format_axes(ax, positions, title)
        self._save(fig, output_name, output_format)

    def _layout(self, roots: List[TreeNode]) -> Tuple[
        Dict[int, Tuple[float, float]],
        List[Tuple[int, int]],
        Dict[int, str],
        Dict[int, int],
        int,
    ]:
        positions: Dict[int, Tuple[float, float]] = {}
        base_positions: Dict[int, Tuple[float, float]] = {}
        edges: List[Tuple[int, int]] = []
        name_map: Dict[int, str] = {}
        depth_map: Dict[int, int] = {}
        x_counter = 0.0
        max_depth = 0

        def walk(node: TreeNode, depth: int) -> float:
            nonlocal x_counter
            nonlocal max_depth
            if not node.children:
                x = x_counter
                x_counter += 1.0
            else:
                child_xs = []
                for child in node.children:
                    child_xs.append(walk(child, depth + 1))
                    edges.append((node.id, child.id))
                x = (min(child_xs) + max(child_xs)) / 2.0

            max_depth = max(max_depth, depth)
            base_positions[node.id] = (x, depth)
            name_map[node.id] = node.name
            depth_map[node.id] = depth
            return x

        for index, root in enumerate(roots):
            walk(root, 0)
            if index < len(roots) - 1:
                x_counter += 1.0

        if self.layout == "vertical":
            for node_id, (x, depth) in base_positions.items():
                positions[node_id] = (
                    x * self.horizontal_spacing,
                    -depth * self.vertical_spacing,
                )
        else:
            for node_id, (x, depth) in base_positions.items():
                positions[node_id] = (
                    depth * self.horizontal_spacing,
                    -x * self.vertical_spacing,
                )

        return positions, edges, name_map, depth_map, max_depth

    def _format_axes(self, ax, positions: Dict[int, Tuple[float, float]], title: str) -> None:
        xs = [pos[0] for pos in positions.values()]
        ys = [pos[1] for pos in positions.values()]
        if not xs or not ys:
            return

        x_pad = max(self.horizontal_spacing, 1.0)
        y_pad = max(self.vertical_spacing, 1.0)
        ax.set_xlim(min(xs) - x_pad, max(xs) + x_pad)
        ax.set_ylim(min(ys) - y_pad, max(ys) + y_pad)
        ax.set_title(title, fontsize=14, weight="bold")
        ax.axis("off")
        fig = ax.get_figure()
        fig.tight_layout()

    def _save(self, fig, output_name: str, output_format: str) -> None:
        os.makedirs(self.output_dir, exist_ok=True)
        output_format = str(output_format or "png").strip().lower()
        if output_format not in {"png", "svg", "pdf"}:
            output_format = "png"
        filename = f"{output_name}.{output_format}"
        output_path = os.path.join(self.output_dir, filename)
        fig.savefig(output_path, dpi=self.dpi)
        plt.close(fig)
        print(f"Saved tree chart to: {output_path}")

    def _color_for_depth(self, depth: int, max_depth: int) -> str:
        if max_depth <= 0:
            return self.root_color
        t = depth / max_depth
        start = mpl_colors.to_rgb(self.root_color)
        end = mpl_colors.to_rgb(self.background_color)
        color = tuple((1 - t) * s + t * e for s, e in zip(start, end))
        return mpl_colors.to_hex(color)

    def _node_size_for_depth(self, depth: int, max_depth: int) -> float:
        if max_depth <= 0:
            return self.node_size
        if depth == 0:
            return self.node_size * self.root_node_scale
        t = depth / max_depth
        scale = max(self.min_node_scale, 1.0 - t * (1.0 - self.min_node_scale))
        return self.node_size * scale

    def _label_size_for_depth(self, depth: int, max_depth: int) -> float:
        if max_depth <= 0:
            return float(self.label_font_size) * self.label_root_scale
        if depth == 0:
            return float(self.label_font_size) * self.label_root_scale
        t = depth / max_depth
        scale = max(self.label_min_scale, 1.0 - t * (1.0 - self.label_min_scale))
        return float(self.label_font_size) * scale

    def _edge_width_for_depth(self, depth: int, max_depth: int) -> float:
        if max_depth <= 0:
            return 1.6
        min_width = 0.6
        max_width = 1.8
        t = depth / max_depth
        return max(min_width, max_width * (1.0 - t))

    def _apply_color_scheme(self) -> None:
        colors = self.config.colors
        if colors.scheme is None or colors.scheme == "":
            return
        try:
            scheme_key = str(int(colors.scheme))
        except (TypeError, ValueError):
            return
        scheme = colors.schemes.get(scheme_key)
        if scheme is not None:
            self.root_color = str(scheme.root)
            self.edge_color = str(scheme.edge)

    @staticmethod
    def _coerce_float(value: Any, default: float) -> float:
        try:
            return float(value)
        except (TypeError, ValueError):
            return default
