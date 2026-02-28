// infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.cpp
#include "infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp"

#include <string>

namespace formatter_c_config_bridge_v1 {
namespace {

auto BuildFormatterLabel(const char* formatter_name) -> std::string {
  if ((formatter_name == nullptr) || (formatter_name[0] == '\0')) {
    return "unknown";
  }
  return {formatter_name};
}

auto IsSupportedKind(uint32_t report_data_kind, const uint32_t* supported_kinds,
                     size_t supported_kind_count) -> bool {
  if (supported_kind_count == 0U) {
    return true;
  }
  if (supported_kinds == nullptr) {
    return false;
  }

  for (size_t index = 0; index < supported_kind_count; ++index) {
    if (supported_kinds[index] == report_data_kind) {
      return true;
    }
  }
  return false;
}

template <typename ConfigType>
auto ValidateAndGetConfigData(const TtFormatterConfig* formatter_config,
                              uint32_t expected_kind,
                              const char* formatter_name,
                              const ConfigType** out_config,
                              std::string* error_message) -> bool {
  if ((out_config == nullptr) || (error_message == nullptr)) {
    return false;
  }
  *out_config = nullptr;
  error_message->clear();

  if (formatter_config == nullptr) {
    *error_message = "formatter_config must not be null.";
    return false;
  }
  if (formatter_config->structSize != sizeof(TtFormatterConfig)) {
    *error_message = "Invalid TtFormatterConfig structSize.";
    return false;
  }
  if (formatter_config->version != TT_FORMATTER_CONFIG_VIEW_VERSION_V1) {
    *error_message = "Unsupported formatter config view version.";
    return false;
  }
  if (formatter_config->configVersion != TT_FORMATTER_CONFIG_DATA_VERSION_V1) {
    *error_message = "Unsupported formatter config payload version.";
    return false;
  }
  if (formatter_config->configKind != expected_kind) {
    *error_message = "Unexpected formatter config kind for " +
                     BuildFormatterLabel(formatter_name) + " formatter.";
    return false;
  }
  if (formatter_config->configData == nullptr) {
    *error_message = "configData must not be null.";
    return false;
  }
  if (formatter_config->configDataSize != sizeof(ConfigType)) {
    *error_message = "Invalid formatter config payload size for " +
                     BuildFormatterLabel(formatter_name) + " formatter.";
    return false;
  }

  const auto* config_data =
      static_cast<const ConfigType*>(formatter_config->configData);
  if ((config_data->structSize != sizeof(ConfigType)) ||
      (config_data->version != TT_FORMATTER_CONFIG_DATA_VERSION_V1)) {
    *error_message = "Invalid formatter config payload header for " +
                     BuildFormatterLabel(formatter_name) + " formatter.";
    return false;
  }

  *out_config = config_data;
  return true;
}

}  // namespace

auto GetDayMdConfigView(const TtFormatterConfig* formatter_config,
                        const TtDayMdConfigV1** out_config,
                        std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_DAY_MD,
                                  "day markdown", out_config, error_message);
}

auto GetDayTexConfigView(const TtFormatterConfig* formatter_config,
                         const TtDayTexConfigV1** out_config,
                         std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_DAY_TEX, "day latex",
                                  out_config, error_message);
}

auto GetDayTypConfigView(const TtFormatterConfig* formatter_config,
                         const TtDayTypConfigV1** out_config,
                         std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_DAY_TYP, "day typst",
                                  out_config, error_message);
}

auto GetMonthMdConfigView(const TtFormatterConfig* formatter_config,
                          const TtMonthMdConfigV1** out_config,
                          std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_MONTH_MD,
                                  "month markdown", out_config, error_message);
}

auto GetMonthTexConfigView(const TtFormatterConfig* formatter_config,
                           const TtMonthTexConfigV1** out_config,
                           std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_MONTH_TEX,
                                  "month latex", out_config, error_message);
}

auto GetMonthTypConfigView(const TtFormatterConfig* formatter_config,
                           const TtMonthTypConfigV1** out_config,
                           std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_MONTH_TYP,
                                  "month typst", out_config, error_message);
}

auto GetRangeMdConfigView(const TtFormatterConfig* formatter_config,
                          const TtRangeMdConfigV1** out_config,
                          std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_RANGE_MD,
                                  "range markdown", out_config, error_message);
}

auto GetRangeTexConfigView(const TtFormatterConfig* formatter_config,
                           const TtRangeTexConfigV1** out_config,
                           std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_RANGE_TEX,
                                  "range latex", out_config, error_message);
}

auto GetRangeTypConfigView(const TtFormatterConfig* formatter_config,
                           const TtRangeTypConfigV1** out_config,
                           std::string* error_message) -> bool {
  return ValidateAndGetConfigData(formatter_config,
                                  TT_FORMATTER_CONFIG_KIND_RANGE_TYP,
                                  "range typst", out_config, error_message);
}

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ValidateReportDataView(const void* report_data, uint32_t report_data_kind,
                            const uint32_t* supported_kinds,
                            size_t supported_kind_count,
                            const char* formatter_name,
                            const char* expected_kinds_hint,
                            const void** out_native_data,
                            int32_t* out_error_code,
                            std::string* out_error_message) -> bool {
  if ((out_native_data == nullptr) || (out_error_code == nullptr) ||
      (out_error_message == nullptr)) {
    return false;
  }

  *out_native_data = nullptr;
  *out_error_code = TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  out_error_message->clear();

  const std::string kFormatterLabel = BuildFormatterLabel(formatter_name);

  if (report_data == nullptr) {
    *out_error_message = "report_data must not be null.";
    return false;
  }

  const auto* report_data_view =
      static_cast<const TtReportDataView*>(report_data);
  if ((report_data_view->structSize != sizeof(TtReportDataView)) ||
      (report_data_view->version != TT_REPORT_DATA_VIEW_VERSION_CURRENT) ||
      (report_data_view->reportDataVersion != TT_REPORT_DATA_VERSION_V1) ||
      (report_data_view->reportData == nullptr) ||
      (report_data_view->reportDataSize == 0U)) {
    *out_error_message =
        "Invalid report_data view for " + kFormatterLabel + " formatter.";
    return false;
  }

  if (!IsSupportedKind(report_data_kind, supported_kinds,
                       supported_kind_count)) {
    *out_error_code = TT_FORMATTER_STATUS_NOT_SUPPORTED;
    *out_error_message =
        "Unsupported report_data_kind for " + kFormatterLabel + " formatter.";
    if ((expected_kinds_hint != nullptr) && (expected_kinds_hint[0] != '\0')) {
      *out_error_message += " ";
      *out_error_message += expected_kinds_hint;
    }
    return false;
  }

  if (report_data_view->reportDataKind != report_data_kind) {
    *out_error_message =
        "report_data_kind mismatch in " + kFormatterLabel + " formatter.";
    return false;
  }

  *out_native_data = report_data_view->reportData;
  *out_error_code = TT_FORMATTER_STATUS_OK;
  out_error_message->clear();
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace formatter_c_config_bridge_v1
