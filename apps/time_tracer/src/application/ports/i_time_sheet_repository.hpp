// application/ports/i_time_sheet_repository.hpp
#ifndef APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_
#define APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_

#include <vector>

#include "application/importer/model/import_models.hpp"

namespace time_tracer::application::ports {

class ITimeSheetRepository {
 public:
  virtual ~ITimeSheetRepository() = default;

  [[nodiscard]] virtual auto IsDbOpen() const -> bool = 0;
  virtual auto ImportData(const std::vector<DayData>& days,
                          const std::vector<TimeRecordInternal>& records)
      -> void = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_
