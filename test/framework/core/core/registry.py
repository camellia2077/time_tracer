# test/core/registry.py

from ..cases.export import ExportTester
from ..cases.ingest import IngestTester
from ..cases.query import QueryTester
from ..cases.version import VersionChecker
from ..conf.definitions import GlobalConfig, TestContext
from .base import BaseTester, TestCounter


def create_test_suite(
    config: GlobalConfig, context: TestContext, counter: TestCounter
) -> list[BaseTester]:
    """
    工厂函数：根据全局配置组装测试套件列表。
    在此处进行具体的参数注入，保持 Engine 的纯净。
    """
    modules = []

    # 1. Ingest Module
    # 假设这里未来可以根据 config.run_control 判断是否跳过某些模块
    modules.append(IngestTester(counter, 1, context))

    # 2. Query Module
    modules.append(
        QueryTester(
            counter=counter,
            module_order=2,
            context=context,
            generated_db_file_name=config.cli_names.GENERATED_DB_FILE_NAME,
            daily_query_dates=config.test_params.DAILY_QUERY_DATES,
            monthly_query_months=config.test_params.MONTHLY_QUERY_MONTHS,
            weekly_query_weeks=config.test_params.WEEKLY_QUERY_WEEKS,
            yearly_query_years=config.test_params.YEARLY_QUERY_YEARS,
            recent_query_days=config.test_params.RECENT_QUERY_DAYS,
            test_formats=config.test_params.TEST_FORMATS,
        )
    )

    # 3. Export Module
    modules.append(
        ExportTester(
            counter=counter,
            module_order=3,
            context=context,
            generated_db_file_name=config.cli_names.GENERATED_DB_FILE_NAME,
            is_bulk_mode=config.test_params.EXPORT_MODE_IS_BULK,
            specific_dates=config.test_params.SPECIFIC_EXPORT_DATES,
            specific_months=config.test_params.SPECIFIC_EXPORT_MONTHS,
            specific_weeks=config.test_params.SPECIFIC_EXPORT_WEEKS,
            specific_years=config.test_params.SPECIFIC_EXPORT_YEARS,
            recent_export_days=config.test_params.RECENT_EXPORT_DAYS,
            test_formats=config.test_params.TEST_FORMATS,
            export_output_path=config.paths.EXPORT_OUTPUT_DIR,
        )
    )

    # 4. Version Check Module
    modules.append(VersionChecker(counter, 4, context))

    return modules
