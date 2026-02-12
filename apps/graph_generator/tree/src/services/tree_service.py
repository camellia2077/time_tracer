from typing import Optional

from core.config import TreeConfig
from data.sqlite_source import TreeSQLiteSource
from services.tree_builder import TreeBuilder
from services.tree_export_coordinator import TreeExportCoordinator
from services.tree_transformer import TreeTransformer

class TreeService:
    """Coordinates data loading, tree building, transformation, and export."""

    def __init__(self, config: TreeConfig, default_output_dir: str = ""):
        self.config = config
        self.output_dir = config.paths.output_directory.strip() or default_output_dir
        self.data_source = TreeSQLiteSource(config.paths.database)
        self.builder = TreeBuilder()
        self.transformer = TreeTransformer()
        self.export_coordinator = TreeExportCoordinator(config, self.output_dir)

    def generate_tree(
        self,
        root_path_override: Optional[str] = None,
        max_depth_override: Optional[int] = None,
    ) -> None:
        projects = self.data_source.fetch_projects()
        if not projects:
            print("No project data found.")
            return

        durations = self.data_source.fetch_project_durations()
        roots = self.builder.build(projects, durations)
        if not roots:
            print("No root projects found.")
            return

        root_path = self.config.settings.root_path
        if root_path_override is not None:
            root_path = root_path_override.strip()
        if root_path:
            roots = self.transformer.filter_roots_by_path(roots, root_path)
            if not roots:
                print(f"No project found matching path: {root_path}")
                return

        max_depth = self.config.settings.max_depth
        if max_depth_override is not None:
            max_depth = max_depth_override
        if max_depth >= 0:
            roots = self.transformer.prune_to_depth(roots, max_depth)

        self.transformer.accumulate_durations(roots)
        self.export_coordinator.export(roots)
