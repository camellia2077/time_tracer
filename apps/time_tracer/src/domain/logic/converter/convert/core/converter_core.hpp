// domain/logic/converter/convert/core/converter_core.hpp
#ifndef DOMAIN_LOGIC_CONVERTER_CONVERT_CORE_CONVERTER_CORE_H_
#define DOMAIN_LOGIC_CONVERTER_CONVERT_CORE_CONVERTER_CORE_H_

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "domain/model/daily_log.hpp"
#include "domain/types/converter_config.hpp"

class DayProcessor {
 public:
  explicit DayProcessor(const ConverterConfig& config);
  void Process(DailyLog& previous_day, DailyLog& day_to_process);

 private:
  const ConverterConfig& config_;
};

class LogLinker {
 public:
  explicit LogLinker(const ConverterConfig& config);
  void LinkLogs(std::map<std::string, std::vector<DailyLog>>& data_map);
  void LinkFirstDayWithExternalPreviousEvent(
      std::map<std::string, std::vector<DailyLog>>& data_map,
      std::string_view previous_date, std::string_view previous_end_time);

 private:
  const ConverterConfig& config_;

  void ProcessCrossDay(DailyLog& current_day, const DailyLog& prev_day);
  static void RecalculateStats(DailyLog& day);
  static auto FormatTime(std::string_view time_str) -> std::string;
};

#endif  // DOMAIN_LOGIC_CONVERTER_CONVERT_CORE_CONVERTER_CORE_H_
