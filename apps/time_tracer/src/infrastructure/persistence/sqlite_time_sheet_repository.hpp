// infrastructure/persistence/sqlite_time_sheet_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_

#include <string>

#include "application/ports/i_time_sheet_repository.hpp"
#include "infrastructure/persistence/importer/repository.hpp"

namespace infrastructure::persistence {

class SqliteTimeSheetRepository final
    : public time_tracer::application::ports::ITimeSheetRepository {
 public:
  explicit SqliteTimeSheetRepository(const std::string& db_path);

  [[nodiscard]] auto IsDbOpen() const -> bool override;
  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records)
      -> void override;

 private:
  importer::Repository repository_;
};

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_
