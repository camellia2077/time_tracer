// application/ports/i_time_sheet_repository.hpp
#ifndef APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_
#define APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/importer/model/import_models.hpp"

namespace time_tracer::application::ports {

struct PreviousActivityTail {
  std::string date;
  std::string end_time;
};

class ITimeSheetRepository {
 public:
  virtual ~ITimeSheetRepository() = default;

  [[nodiscard]] virtual auto IsDbOpen() const -> bool = 0;
  virtual auto ImportData(const std::vector<DayData>& days,
                          const std::vector<TimeRecordInternal>& records)
      -> void = 0;
  virtual auto ReplaceMonthData(int year, int month,
                                const std::vector<DayData>& days,
                                const std::vector<TimeRecordInternal>& records)
      -> void = 0;
  [[nodiscard]] virtual auto TryGetLatestActivityTailBeforeDate(
      std::string_view date) const -> std::optional<PreviousActivityTail> = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_
