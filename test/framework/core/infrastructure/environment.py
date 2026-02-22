# test/infrastructure/environment.py
from pathlib import Path
from typing import List, Optional

from .workspace import Workspace
from .deployer import ArtifactDeployer


class EnvironmentManager:
    """
    Facade: 供 Engine 调用。
    负责组装 Workspace 和 Deployer 来完成具体的测试环境准备任务。
    """

    def __init__(
        self,
        source_exe_dir: Path,
        files_to_copy: List[str],
        folders_to_copy: List[str],
        use_temp: bool = True,
    ):
        self.source_dir = source_exe_dir
        self.files = files_to_copy
        self.folders = folders_to_copy

        # 组合 Workspace
        self.workspace = Workspace(use_temp=use_temp)
        self.target_root = None

    # [修改] 增加 should_deploy 参数，默认为 True
    def setup(
        self,
        target_dir_override: Optional[Path] = None,
        should_clean: bool = False,
        should_deploy: bool = True,
    ) -> Path:
        # 1. 准备地皮 (清理)
        self.target_root = self.workspace.setup(
            target_dir_override, should_clean=should_clean
        )

        # 2. 搬运物资 (受控执行)
        if should_deploy:
            deployer = ArtifactDeployer(self.source_dir, self.target_root)
            deployer.deploy(self.files, self.folders)

        return self.target_root

    def teardown(self):
        self.workspace.teardown()
