from typing import List

from core.config import TreeConfig
from core.models import TreeNode
from services.tree_exporters import (
    ExportRequest,
    HtmlTreeExporter,
    ImageTreeExporter,
    JsonTreeExporter,
    TreeExporter,
)


class TreeExportCoordinator:
    """Coordinates image/html/json exporters via a common exporter interface."""

    def __init__(self, config: TreeConfig, output_dir: str):
        self.config = config
        self.output_dir = output_dir
        self.image_exporter: TreeExporter = ImageTreeExporter(config, output_dir)
        self.json_exporter: TreeExporter = JsonTreeExporter(output_dir)
        self.html_exporter: TreeExporter = HtmlTreeExporter(output_dir)

    def export(self, roots: List[TreeNode]) -> None:
        request = ExportRequest(
            roots=roots,
            title=self.config.settings.title,
            output_name=self.config.settings.output_name,
        )
        for exporter in self._resolve_exporters():
            exporter.export(request)

    def _resolve_exporters(self) -> List[TreeExporter]:
        settings = self.config.settings
        exporters: List[TreeExporter] = []
        if settings.output_images:
            exporters.append(self.image_exporter)
        if settings.output_json:
            exporters.append(self.json_exporter)
        if settings.output_html:
            exporters.append(self.html_exporter)
        return exporters
