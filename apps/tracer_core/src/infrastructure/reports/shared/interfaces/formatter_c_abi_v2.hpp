// infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_ABI_V2_H_
#define INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_ABI_V2_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTBEGIN(readability-identifier-naming,readability-magic-numbers,modernize-use-using)
// -----------------------------------------------------------------------------
// ABI v2 contract:
// 1) All entry points use C ABI and `tt_` prefix (e.g. `tt_createFormatter`).
// 2) `tt_formatReport` allocates `out_report_content`; caller must release it
// by
//    invoking `tt_freeCString` from the same plugin.
// 3) Version and error information are exposed through fixed-size structs.
// -----------------------------------------------------------------------------

#define TT_FORMATTER_ABI_VERSION_V2 2U
#define TT_FORMATTER_ABI_VERSION_V3 3U
#define TT_FORMATTER_ABI_VERSION_V4 4U
#define TT_FORMATTER_ABI_VERSION_V5 5U
#define TT_FORMATTER_ABI_VERSION_CURRENT TT_FORMATTER_ABI_VERSION_V5

#define TT_FORMATTER_CONFIG_VIEW_VERSION_V1 1U
#define TT_FORMATTER_CONFIG_DATA_VERSION_V1 1U
#define TT_FORMATTER_CONFIG_VERSION_V1 TT_FORMATTER_CONFIG_VIEW_VERSION_V1
#define TT_REPORT_DATA_VIEW_VERSION_V1 1U
#define TT_REPORT_DATA_VIEW_VERSION_V2 2U
#define TT_REPORT_DATA_VIEW_VERSION_CURRENT TT_REPORT_DATA_VIEW_VERSION_V2
#define TT_REPORT_DATA_VERSION_V1 1U

enum TtFormatterStatusCode {
  TT_FORMATTER_STATUS_OK = 0,
  TT_FORMATTER_STATUS_INVALID_ARGUMENT = 1,
  TT_FORMATTER_STATUS_CONFIG_ERROR = 2,
  TT_FORMATTER_STATUS_FORMAT_ERROR = 3,
  TT_FORMATTER_STATUS_MEMORY_ERROR = 4,
  TT_FORMATTER_STATUS_NOT_SUPPORTED = 5,
  TT_FORMATTER_STATUS_INTERNAL_ERROR = 100
};

enum TtReportDataKind {
  TT_REPORT_DATA_KIND_UNKNOWN = 0,
  TT_REPORT_DATA_KIND_DAILY = 1,
  TT_REPORT_DATA_KIND_MONTHLY = 2,
  TT_REPORT_DATA_KIND_PERIOD = 3,
  TT_REPORT_DATA_KIND_RANGE = 4,
  TT_REPORT_DATA_KIND_WEEKLY = 5,
  TT_REPORT_DATA_KIND_YEARLY = 6
};

typedef void* TtFormatterHandle;

typedef struct TtFormatterVersion {
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
  uint16_t reserved;
} TtFormatterVersion;

typedef struct TtFormatterAbiInfo {
  uint32_t structSize;
  uint32_t abiVersion;
  TtFormatterVersion implementationVersion;
} TtFormatterAbiInfo;

typedef struct TtFormatterError {
  uint32_t structSize;
  int32_t code;
  const char* message;
} TtFormatterError;

typedef struct TtStringView {
  const char* data;
  uint64_t length;
} TtStringView;

enum TtFormatterConfigKind {
  TT_FORMATTER_CONFIG_KIND_UNKNOWN = 0,
  TT_FORMATTER_CONFIG_KIND_DAY_MD = 101,
  TT_FORMATTER_CONFIG_KIND_DAY_TEX = 102,
  TT_FORMATTER_CONFIG_KIND_DAY_TYP = 103,
  TT_FORMATTER_CONFIG_KIND_MONTH_MD = 201,
  TT_FORMATTER_CONFIG_KIND_MONTH_TEX = 202,
  TT_FORMATTER_CONFIG_KIND_MONTH_TYP = 203,
  TT_FORMATTER_CONFIG_KIND_RANGE_MD = 301,
  TT_FORMATTER_CONFIG_KIND_RANGE_TEX = 302,
  TT_FORMATTER_CONFIG_KIND_RANGE_TYP = 303
};

typedef struct TtFormatterKeywordColorV1 {
  TtStringView keyword;
  TtStringView color;
} TtFormatterKeywordColorV1;

typedef struct TtFormatterStatisticItemNodeV1 {
  TtStringView label;
  TtStringView dbColumn;
  uint8_t show;
  uint8_t reserved0[3];
  int32_t parentIndex;
} TtFormatterStatisticItemNodeV1;

typedef struct TtDayLabelsConfigV1 {
  TtStringView titlePrefix;
  TtStringView reportTitle;
  TtStringView dateLabel;
  TtStringView totalTimeLabel;
  TtStringView statusLabel;
  TtStringView sleepLabel;
  TtStringView getupTimeLabel;
  TtStringView remarkLabel;
  TtStringView exerciseLabel;
  TtStringView noRecordsMessage;
  TtStringView statisticsLabel;
  TtStringView allActivitiesLabel;
  TtStringView activityRemarkLabel;
  TtStringView activityConnector;
  TtStringView projectBreakdownLabel;
} TtDayLabelsConfigV1;

typedef struct TtMonthLabelsConfigV1 {
  TtStringView reportTitle;
  TtStringView titleTemplate;
  TtStringView actualDaysLabel;
  TtStringView statusDaysLabel;
  TtStringView sleepDaysLabel;
  TtStringView exerciseDaysLabel;
  TtStringView cardioDaysLabel;
  TtStringView anaerobicDaysLabel;
  TtStringView totalTimeLabel;
  TtStringView noRecordsMessage;
  TtStringView invalidFormatMessage;
  TtStringView projectBreakdownLabel;
} TtMonthLabelsConfigV1;

typedef struct TtRangeLabelsConfigV1 {
  TtStringView titleTemplate;
  TtStringView actualDaysLabel;
  TtStringView statusDaysLabel;
  TtStringView sleepDaysLabel;
  TtStringView exerciseDaysLabel;
  TtStringView cardioDaysLabel;
  TtStringView anaerobicDaysLabel;
  TtStringView totalTimeLabel;
  TtStringView noRecordsMessage;
  TtStringView invalidRangeMessage;
  TtStringView projectBreakdownLabel;
} TtRangeLabelsConfigV1;

typedef struct TtTexStyleConfigV1 {
  TtStringView mainFont;
  TtStringView cjkMainFont;
  int32_t baseFontSize;
  int32_t reportTitleFontSize;
  int32_t categoryTitleFontSize;
  double marginIn;
  double listTopSepPt;
  double listItemSepEx;
} TtTexStyleConfigV1;

typedef struct TtTypstStyleConfigV1 {
  TtStringView baseFont;
  TtStringView titleFont;
  TtStringView categoryTitleFont;
  int32_t baseFontSize;
  int32_t reportTitleFontSize;
  int32_t categoryTitleFontSize;
  double lineSpacingEm;
  double marginTopCm;
  double marginBottomCm;
  double marginLeftCm;
  double marginRightCm;
} TtTypstStyleConfigV1;

typedef struct TtDayMdConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtDayLabelsConfigV1 labels;
  const TtFormatterStatisticItemNodeV1* statisticsItems;
  uint32_t statisticsItemCount;
  uint32_t reserved;
} TtDayMdConfigV1;

typedef struct TtDayTexConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtDayLabelsConfigV1 labels;
  TtTexStyleConfigV1 style;
  const TtFormatterKeywordColorV1* keywordColors;
  uint32_t keywordColorCount;
  const TtFormatterStatisticItemNodeV1* statisticsItems;
  uint32_t statisticsItemCount;
} TtDayTexConfigV1;

typedef struct TtDayTypConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtDayLabelsConfigV1 labels;
  TtTypstStyleConfigV1 style;
  const TtFormatterKeywordColorV1* keywordColors;
  uint32_t keywordColorCount;
  const TtFormatterStatisticItemNodeV1* statisticsItems;
  uint32_t statisticsItemCount;
  int32_t statisticFontSize;
  int32_t statisticTitleFontSize;
} TtDayTypConfigV1;

typedef struct TtMonthMdConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtMonthLabelsConfigV1 labels;
} TtMonthMdConfigV1;

typedef struct TtMonthTexConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtMonthLabelsConfigV1 labels;
  TtTexStyleConfigV1 style;
} TtMonthTexConfigV1;

typedef struct TtMonthTypConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtMonthLabelsConfigV1 labels;
  TtTypstStyleConfigV1 style;
} TtMonthTypConfigV1;

typedef struct TtRangeMdConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtRangeLabelsConfigV1 labels;
} TtRangeMdConfigV1;

typedef struct TtRangeTexConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtRangeLabelsConfigV1 labels;
  TtTexStyleConfigV1 style;
} TtRangeTexConfigV1;

typedef struct TtRangeTypConfigV1 {
  uint32_t structSize;
  uint32_t version;
  TtRangeLabelsConfigV1 labels;
  TtTypstStyleConfigV1 style;
} TtRangeTypConfigV1;

typedef struct TtFormatterConfig {
  uint32_t structSize;
  uint32_t version;
  uint32_t configKind;
  uint32_t configVersion;
  const void* configData;
  uint64_t configDataSize;
} TtFormatterConfig;

typedef struct TtDayMetadataV1 {
  TtStringView status;
  TtStringView sleep;
  TtStringView remark;
  TtStringView getupTime;
  TtStringView exercise;
} TtDayMetadataV1;

typedef struct TtDailyTimeRecordV1 {
  TtStringView startTime;
  TtStringView endTime;
  TtStringView projectPath;
  int64_t durationSeconds;
  TtStringView activityRemark;
  uint8_t hasActivityRemark;
  uint8_t reserved[7];
} TtDailyTimeRecordV1;

typedef struct TtStringInt64PairV1 {
  TtStringView key;
  int64_t value;
} TtStringInt64PairV1;

typedef struct TtProjectTreeNodeV1 {
  TtStringView name;
  int64_t duration;
  int32_t parentIndex;
  uint32_t reserved;
} TtProjectTreeNodeV1;

typedef struct TtDailyReportDataV1 {
  uint32_t structSize;
  uint32_t version;
  TtStringView date;
  TtDayMetadataV1 metadata;
  int64_t totalDuration;
  const TtDailyTimeRecordV1* detailedRecords;
  uint32_t detailedRecordCount;
  const TtStringInt64PairV1* stats;
  uint32_t statsCount;
  const TtProjectTreeNodeV1* projectTreeNodes;
  uint32_t projectTreeNodeCount;
  uint32_t reserved;
} TtDailyReportDataV1;

typedef struct TtRangeReportDataV1 {
  uint32_t structSize;
  uint32_t version;
  TtStringView rangeLabel;
  TtStringView startDate;
  TtStringView endDate;
  int32_t requestedDays;
  int64_t totalDuration;
  int32_t actualDays;
  int32_t statusTrueDays;
  int32_t sleepTrueDays;
  int32_t exerciseTrueDays;
  int32_t cardioTrueDays;
  int32_t anaerobicTrueDays;
  uint8_t isValid;
  uint8_t reserved0[3];
  const TtProjectTreeNodeV1* projectTreeNodes;
  uint32_t projectTreeNodeCount;
  uint32_t reserved;
} TtRangeReportDataV1;

typedef TtRangeReportDataV1 TtMonthlyReportDataV1;
typedef TtRangeReportDataV1 TtPeriodReportDataV1;
typedef TtRangeReportDataV1 TtWeeklyReportDataV1;
typedef TtRangeReportDataV1 TtYearlyReportDataV1;

typedef struct TtReportDataView {
  uint32_t structSize;
  uint32_t version;
  uint32_t reportDataKind;
  uint32_t reportDataVersion;
  const void* reportData;
  uint64_t reportDataSize;
} TtReportDataView;

using TtGetFormatterAbiInfoFuncV2 = int32_t (*)(TtFormatterAbiInfo* out_abi);
using TtCreateFormatterFuncV2 = int32_t (*)(const TtFormatterConfig* config,
                                            TtFormatterHandle* out_handle);
using TtDestroyFormatterFuncV2 = int32_t (*)(TtFormatterHandle handle);
using TtFormatReportFuncV2 = int32_t (*)(TtFormatterHandle handle,
                                         const void* report_data,
                                         uint32_t report_data_kind,
                                         char** out_report_content,
                                         uint64_t* out_report_size);
using TtFreeCStringFuncV2 = void (*)(char* c_string);
using TtGetLastErrorFuncV2 = int32_t (*)(TtFormatterHandle handle,
                                         TtFormatterError* out_error);
// NOLINTEND(readability-identifier-naming,readability-magic-numbers,modernize-use-using)

#ifdef __cplusplus
}
#endif

#endif  // INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_ABI_V2_H_
