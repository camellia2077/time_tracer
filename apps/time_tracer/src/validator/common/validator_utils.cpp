// validator/common/validator_utils.cpp
#include "validator/common/validator_utils.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "common/ansi_colors.hpp"

namespace validator {

static auto GetErrorTypeHeader(ErrorType type) -> std::string {
  switch (type) {
    case ErrorType::kSourceRemarkAfterEvent:
    case ErrorType::kSourceNoDateAtStart:
    case ErrorType::kSourceInvalidLineFormat:
    case ErrorType::kUnrecognizedActivity:
    case ErrorType::kSourceMissingYearHeader:
      return "Source file format errors (源文件格式错误):";
    case ErrorType::kIncorrectDayCountForMonth:
      return "Date Logic errors(日期逻辑错误):";
    case ErrorType::kDateContinuity:
      return "Date Continuity errors (日期中断/缺失):";
    case ErrorType::kTimeDiscontinuity:
      return "Activity Time Discontinuity errors (活动时间不连续):";
    case ErrorType::kMissingSleepNight:
      return "Lack of sleep activity errors(最后的活动项目缺少sleep活动):";
    case ErrorType::kJsonTooFewActivities:
      return "Activity count errors(活动数量错误):";
    case ErrorType::kFileAccess:
      return "File access errors:";
    case ErrorType::kStructural:
      return "Structural errors:";
    case ErrorType::kLineFormat:
      return "Line format errors:";
    case ErrorType::kLogical:
      return "Logical consistency errors:";
    default:
      return "Other errors:";
  }
}

void PrintGroupedErrors(const std::string& filename,
                        const std::set<Error>& errors) {
  std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
  std::map<ErrorType, std::vector<Error>> grouped_errors;
  for (const auto& err : errors) {
    grouped_errors[err.type].push_back(err);
  }

  const std::string kErrorLogPath =
      "./output/errors.log";  // 建议后续改为可配置路径
  std::ofstream err_stream(kErrorLogPath, std::ios::app);

  err_stream
      << "\n文件 " << filename
      << " 的检验错误\n--------------------------------------------------\n\n";

  for (const auto& pair : grouped_errors) {
    std::string header = GetErrorTypeHeader(pair.first);
    std::cerr << "\n" << header << std::endl;
    err_stream << header << "\n";
    for (const auto& err : pair.second) {
      std::string error_message =
          (err.line_number == 0)
              ? err.message
              : "Line " + std::to_string(err.line_number) + ": " + err.message;
      std::cerr << "  " << error_message << std::endl;
      err_stream << "  " << error_message << "\n";
    }
  }
  err_stream.close();
  std::cout << "\n详细的错误日志已保存至: " << time_tracer::common::colors::kYellow << kErrorLogPath
            << time_tracer::common::colors::kReset << std::endl;
}

}  // namespace validator