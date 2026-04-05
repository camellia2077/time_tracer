#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_INGEST_SYNC_SQL_HPP_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_INGEST_SYNC_SQL_HPP_

#include <optional>
#include <string>

#include <sqlite3.h>

#include "application/dto/pipeline_responses.hpp"

namespace tracer::core::infrastructure::persistence::importer::detail {

struct MonthBoundary {
  std::string start_date;
  std::string next_month_start_date;
};

[[nodiscard]] auto BuildMonthBoundary(int year, int month)
    -> std::optional<MonthBoundary>;

auto UpsertIngestSyncStatusRow(
    sqlite3* sqlite_db,
    const tracer_core::core::dto::IngestSyncStatusEntry& entry) -> void;

[[nodiscard]] auto ListIngestSyncStatusRows(
    sqlite3* sqlite_db,
    const tracer_core::core::dto::IngestSyncStatusRequest& request)
    -> tracer_core::core::dto::IngestSyncStatusOutput;

}  // namespace tracer::core::infrastructure::persistence::importer::detail

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_INGEST_SYNC_SQL_HPP_
