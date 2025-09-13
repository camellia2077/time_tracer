# graph_generator/data/day_analyzer.py
import pandas as pd
from datetime import datetime, timedelta

# --- [核心修改] 将相对导入改为绝对导入 ---
from data import db_access 
from core.config import COLOR_RED, COLOR_RESET

class LogicalDay:
    """代表并处理单个逻辑日的所有数据。"""
    def __init__(self, raw_records, getup_time_str, parent_lookup):
        if raw_records.empty:
            raise ValueError("不能从空的记录创建 LogicalDay。")
        self.raw_records = raw_records.copy()
        self.getup_time_str = getup_time_str
        self.parent_lookup = parent_lookup
        self._parent_cache = {}
        self.start_time = None
        self.end_time = None
        self.processed_data = None
        self._process()

    def _get_ultimate_parent(self, path):
        if path in self._parent_cache:
            return self._parent_cache[path]
        original_path = path
        for _ in range(10): # 避免无限循环
            parent = self.parent_lookup.get(path)
            if parent is None or parent == path:
                self._parent_cache[original_path] = path
                return path
            path = parent
        self._parent_cache[original_path] = path 
        return path

    def _process(self):
        base_date_str = self.raw_records['date'].iloc[0]
        self.start_time = datetime.strptime(f"{base_date_str} {self.getup_time_str}", "%Y%m%d %H:%M")
        self._correct_timestamps()
        self._determine_logical_end()
        self._filter_records()
        if not self.processed_data.empty:
            self.processed_data['parent'] = self.processed_data['project_path'].apply(self._get_ultimate_parent)

    def _correct_timestamps(self):
        self.raw_records['start_dt'] = self.raw_records.apply(lambda r: datetime.strptime(f"{r['date']} {r['start']}", "%Y%m%d %H:%M"), axis=1)
        self.raw_records['end_dt'] = self.raw_records.apply(lambda r: datetime.strptime(f"{r['date']} {r['end']}", "%Y%m%d %H:%M"), axis=1)
        is_next_day = (self.raw_records['start_dt'] < self.raw_records['start_dt'].shift(1).fillna(pd.Timestamp.min))
        day_offset = is_next_day.cumsum()
        self.raw_records['start_dt'] += pd.to_timedelta(day_offset, unit='D')
        self.raw_records['end_dt'] += pd.to_timedelta(day_offset, unit='D')
        overnight_mask = self.raw_records['end_dt'] < self.raw_records['start_dt']
        self.raw_records.loc[overnight_mask, 'end_dt'] += timedelta(days=1)

    def _determine_logical_end(self):
        # 注意：原版逻辑中的 sleep_night 可能需要调整为通用的 sleep
        sleep_night_record = self.raw_records[
            (self.raw_records['project_path'] == 'sleep') & 
            (self.raw_records['start_dt'] >= self.start_time)
        ]
        if sleep_night_record.empty:
            self.end_time = self.raw_records['end_dt'].max()
        else:
            self.end_time = sleep_night_record['end_dt'].iloc[0]

    def _filter_records(self):
        self.processed_data = self.raw_records[
            (self.raw_records['start_dt'] >= self.start_time) &
            (self.raw_records['end_dt'] <= self.end_time)
        ].copy()

class DataProcessor:
    """使用 db_access 模块创建 LogicalDay 对象。"""
    def __init__(self):
        db_data = db_access.get_data_for_timeline()
        if db_data is None:
            raise ConnectionError("无法从数据库加载数据。")
        self.df_days, self.df_records, self.df_parents = db_data
        self.parent_lookup = pd.Series(self.df_parents.parent.values, index=self.df_parents.child).to_dict()

    def create_logical_day(self, target_date_str):
        day_info = self.df_days[self.df_days['date'] == target_date_str]
        if day_info.empty:
            print(f"{COLOR_RED}错误: 未找到日期 {target_date_str} 的数据。{COLOR_RESET}")
            return None
        getup_time_str = day_info['getup_time'].iloc[0]
        raw_records = self.df_records[self.df_records['date'] == target_date_str]
        if raw_records.empty:
            print(f"{COLOR_RED}错误: 未找到 {target_date_str} 的时间记录。{COLOR_RESET}")
            return None
        return LogicalDay(raw_records, getup_time_str, self.parent_lookup)