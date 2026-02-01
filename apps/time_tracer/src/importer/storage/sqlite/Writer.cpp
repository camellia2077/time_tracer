// importer/storage/sqlite/Writer.cpp
#include "writer.hpp"

#include <iostream>

namespace {
// Day Table Bind Indices
constexpr int kDayIdxDate = 1;
constexpr int kDayIdxYear = 2;
constexpr int kDayIdxMonth = 3;
constexpr int kDayIdxStatus = 4;
constexpr int kDayIdxSleep = 5;
constexpr int kDayIdxRemark = 6;
constexpr int kDayIdxGetupTime = 7;
constexpr int kDayIdxExercise = 8;
constexpr int kDayIdxExerciseTime = 9;
constexpr int kDayIdxCardioTime = 10;
constexpr int kDayIdxAnaerobicTime = 11;
constexpr int kDayIdxGamingTime = 12;
constexpr int kDayIdxGroomingTime = 13;
constexpr int kDayIdxToiletTime = 14;
constexpr int kDayIdxSleepNightTime = 15;
constexpr int kDayIdxSleepDayTime = 16;
constexpr int kDayIdxSleepTotalTime = 17;
constexpr int kDayIdxRecreationTime = 18;
constexpr int kDayIdxRecreationZhihuTime = 19;
constexpr int kDayIdxRecreationBilibiliTime = 20;
constexpr int kDayIdxRecreationDouyinTime = 21;
constexpr int kDayIdxStudyTime = 22;

// Record Table Bind Indices
constexpr int kRecordIdxLogicalId = 1;
constexpr int kRecordIdxStartTimestamp = 2;
constexpr int kRecordIdxEndTimestamp = 3;
constexpr int kRecordIdxDate = 4;
constexpr int kRecordIdxStartTimeStr = 5;
constexpr int kRecordIdxEndTimeStr = 6;
constexpr int kRecordIdxProjectId = 7;
constexpr int kRecordIdxDuration = 8;
constexpr int kRecordIdxRemark = 9;
}  // namespace

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
Writer::Writer(sqlite3* sqlite_db, sqlite3_stmt* stmt_day,
               sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_insert_project)
    : db_(sqlite_db),
      stmt_insert_day_(stmt_day),
      stmt_insert_record_(stmt_record),
      stmt_insert_project_(stmt_insert_project) {
  // [核心修改] 初始化解析器委托对象
  project_resolver_ =
      std::make_unique<ProjectResolver>(db_, stmt_insert_project_);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

Writer::~Writer() = default;

void Writer::insert_days(const std::vector<DayData>& days) {
  // 这部分逻辑未变，保持原样即可
  for (const auto& day_data : days) {
    sqlite3_bind_text(stmt_insert_day_, kDayIdxDate, day_data.date.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxYear, day_data.year);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxMonth, day_data.month);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxStatus, day_data.status);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxSleep, day_data.sleep);
    sqlite3_bind_text(stmt_insert_day_, kDayIdxRemark, day_data.remark.c_str(),
                      -1, SQLITE_TRANSIENT);

    if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
      sqlite3_bind_null(stmt_insert_day_, kDayIdxGetupTime);
    } else {
      sqlite3_bind_text(stmt_insert_day_, kDayIdxGetupTime,
                        day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt_insert_day_, kDayIdxExercise, day_data.exercise);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxExerciseTime,
                     day_data.stats.total_exercise_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxCardioTime,
                     day_data.stats.cardio_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxAnaerobicTime,
                     day_data.stats.anaerobic_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxGamingTime,
                     day_data.stats.gaming_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxGroomingTime,
                     day_data.stats.grooming_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxToiletTime,
                     day_data.stats.toilet_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxSleepNightTime,
                     day_data.stats.sleep_night_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxSleepDayTime,
                     day_data.stats.sleep_day_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxSleepTotalTime,
                     day_data.stats.sleep_total_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxRecreationTime,
                     day_data.stats.recreation_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxRecreationZhihuTime,
                     day_data.stats.recreation_zhihu_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxRecreationBilibiliTime,
                     day_data.stats.recreation_bilibili_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxRecreationDouyinTime,
                     day_data.stats.recreation_douyin_time);
    sqlite3_bind_int(stmt_insert_day_, kDayIdxStudyTime,
                     day_data.stats.study_time);

    if (sqlite3_step(stmt_insert_day_) != SQLITE_DONE) {
      std::cerr << "Error inserting day row: " << sqlite3_errmsg(db_)
                << std::endl;
    }
    sqlite3_reset(stmt_insert_day_);
  }
}

void Writer::insert_records(const std::vector<TimeRecordInternal>& records) {
  if (records.empty()) {
    return;
  }

  // 1. 收集需要解析的路径 (Adapter: Struct -> String)
  std::vector<std::string> paths;
  paths.reserve(records.size());
  for (const auto& record : records) {
    paths.push_back(record.project_path);
  }

  // 2. 委托 Resolver 批量预处理 (建立缓存)
  project_resolver_->preload_and_resolve(paths);

  // 3. 极速插入循环
  for (const auto& record_data : records) {
    // 直接从 Resolver 获取 ID
    long long project_id = project_resolver_->get_id(record_data.project_path);

    sqlite3_bind_int64(stmt_insert_record_, kRecordIdxLogicalId,
                       record_data.logical_id);
    sqlite3_bind_int64(stmt_insert_record_, kRecordIdxStartTimestamp,
                       record_data.start_timestamp);
    sqlite3_bind_int64(stmt_insert_record_, kRecordIdxEndTimestamp,
                       record_data.end_timestamp);
    sqlite3_bind_text(stmt_insert_record_, kRecordIdxDate,
                      record_data.date.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_text(stmt_insert_record_, kRecordIdxStartTimeStr,
                      record_data.start_time_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert_record_, kRecordIdxEndTimeStr,
                      record_data.end_time_str.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int64(stmt_insert_record_, kRecordIdxProjectId, project_id);
    sqlite3_bind_int(stmt_insert_record_, kRecordIdxDuration,
                     record_data.duration_seconds);

    if (record_data.remark.has_value()) {
      sqlite3_bind_text(stmt_insert_record_, kRecordIdxRemark,
                        record_data.remark->c_str(), -1, SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt_insert_record_, kRecordIdxRemark);
    }

    if (sqlite3_step(stmt_insert_record_) != SQLITE_DONE) {
      std::cerr << "Error inserting record row: " << sqlite3_errmsg(db_)
                << std::endl;
    }
    sqlite3_reset(stmt_insert_record_);
  }
}
