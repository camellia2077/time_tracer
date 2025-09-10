# _py_internal/environment_manager.py
import sys
import shutil
from pathlib import Path

class EnvironmentManager:
    """负责测试环境的准备和清理工作。"""
    
    def __init__(self, config):
        """
        初始化环境管理器。
        :param config: 导入的 config 模块，其中包含所有路径和设置。
        """
        self.config = config
        self.root_dir = Path.cwd()

    def setup(self):
        """执行完整的环境设置过程：清理、创建目录和复制文件。"""
        print(f"{self.config.Colors.CYAN}--- 1. Cleaning Artifacts & Setting up Directories ---{self.config.Colors.RESET}")
        self._clean_directories()
        self._clean_files()
        self._create_output_dirs()
        print(f"  {self.config.Colors.GREEN}清理完成，已创建 '{self.config.Paths.OUTPUT_DIR_NAME}' 和 'py_output' 目录。{self.config.Colors.RESET}")

        print(f"\n{self.config.Colors.CYAN}--- 2. Preparing Executable, DLLs and Config ---{self.config.Colors.RESET}")
        if not self.config.Paths.SOURCE_EXECUTABLES_DIR.exists():
            print(f"  {self.config.Colors.RED}错误: 源目录不存在: {self.config.Paths.SOURCE_EXECUTABLES_DIR}{self.config.Colors.RESET}")
            sys.exit(1)
        
        self._copy_artifacts()
        self._copy_config_folder()
        print("  可执行文件、DLL和配置已准备就绪。")

    def _clean_directories(self):
        """清理需要移除的目录。"""
        for dir_name in self.config.DIRECTORIES_TO_CLEAN:
            dir_path = self.root_dir / dir_name
            if dir_path.exists():
                try:
                    shutil.rmtree(dir_path)
                    print(f"  {self.config.Colors.GREEN}已移除旧目录: {dir_name}{self.config.Colors.RESET}")
                except OSError as e:
                    print(f"  {self.config.Colors.RED}移除目录 '{dir_name}' 时出错: {e}{self.config.Colors.RESET}")
                    sys.exit(1)
    
    def _clean_files(self):
        """清理需要移除的旧文件（EXE, DLLs, DB, Logs）。"""
        # [核心修改] 合并两个列表进行清理，以确保所有旧文件都被删除
        all_files_to_clean = self.config.FILES_TO_CLEAN + self.config.FILES_TO_COPY
        for file_name in all_files_to_clean:
            file_path = self.config.Paths.TARGET_EXECUTABLES_DIR / file_name
            if file_path.exists():
                try:
                    file_path.unlink()
                    print(f"  {self.config.Colors.GREEN}已移除旧文件: {file_name}{self.config.Colors.RESET}")
                except OSError as e:
                    print(f"  {self.config.Colors.RED}移除文件 '{file_name}' 时出错: {e}{self.config.Colors.RESET}")
                    sys.exit(1)

    def _create_output_dirs(self):
        """创建本次运行所需的输出目录。"""
        (self.root_dir / self.config.Paths.OUTPUT_DIR_NAME).mkdir(parents=True, exist_ok=True)
        (self.root_dir / "py_output").mkdir(parents=True, exist_ok=True)

    def _copy_artifacts(self):
        """复制可执行文件和所有依赖的DLL。"""
        # [核心修改] 只遍历 FILES_TO_COPY 列表
        for artifact_name in self.config.FILES_TO_COPY:
            source_path = self.config.Paths.SOURCE_EXECUTABLES_DIR / artifact_name
            target_path = self.config.Paths.TARGET_EXECUTABLES_DIR / artifact_name
            if not source_path.exists():
                print(f"  {self.config.Colors.RED}警告: 在源目录中未找到文件: {artifact_name}{self.config.Colors.RESET}")
                continue
            try:
                shutil.copy(source_path, target_path)
                print(f"  {self.config.Colors.GREEN}已成功复制: {artifact_name}{self.config.Colors.RESET}")
            except Exception as e:
                print(f"  {self.config.Colors.RED}复制文件时出错 {artifact_name}: {e}{self.config.Colors.RESET}")
                sys.exit(1)

    def _copy_config_folder(self):
        """复制配置文件所在的 'config' 文件夹。"""
        source_config_path = self.config.Paths.SOURCE_EXECUTABLES_DIR / "config"
        target_config_path = self.config.Paths.TARGET_EXECUTABLES_DIR / "config"
        if source_config_path.exists() and source_config_path.is_dir():
            try:
                if target_config_path.exists():
                    shutil.rmtree(target_config_path)
                shutil.copytree(source_config_path, target_config_path)
                print(f"  {self.config.Colors.GREEN}已成功复制: config 文件夹{self.config.Colors.RESET}")
            except Exception as e:
                print(f"  {self.config.Colors.RED}复制 config 文件夹时出错: {e}{self.config.Colors.RESET}")
                sys.exit(1)
        else:
            print(f"  {self.config.Colors.RED}警告: 在源目录中未找到 config 文件夹，跳过复制。{self.config.Colors.RESET}")