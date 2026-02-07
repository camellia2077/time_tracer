// infrastructure/config/internal/config_parser_utils.cpp
#include "infrastructure/config/internal/config_parser_utils.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace ConfigParserUtils {

namespace {
auto ResolveDefaultPath(const fs::path& exe_path, const std::string& path_value)
    -> fs::path {
  fs::path path = path_value;
  if (path.is_relative()) {
    return exe_path / path;
  }
  return path;
}

auto ParseDateCheckMode(std::string_view mode_str) -> DateCheckMode {
  if (mode_str == "none") {
    return DateCheckMode::kNone;
  }
  if (mode_str == "continuity") {
    return DateCheckMode::kContinuity;
  }
  if (mode_str == "full") {
    return DateCheckMode::kFull;
  }
  throw std::runtime_error("Invalid date_check mode: " + std::string(mode_str));
}
auto ParseGlobalDefaults(const toml::table& defaults_tbl,
                         const fs::path& exe_path, AppConfig& config) -> void {
  if (auto val = defaults_tbl.get("db_path")->value<std::string>()) {
    config.defaults.db_path = ResolveDefaultPath(exe_path, *val);
  }
  if (auto val = defaults_tbl.get("output_root")->value<std::string>()) {
    config.defaults.output_root = ResolveDefaultPath(exe_path, *val);
  }
  if (auto val = defaults_tbl.get("default_format")->value<std::string>()) {
    config.defaults.default_format = *val;
  }
}

auto ParseExportDefaults(const toml::table& export_tbl, AppConfig& config)
    -> void {
  if (auto val = export_tbl.get("format")->value<std::string>()) {
    config.command_defaults.export_format = *val;
  }
}

auto ParseConvertDefaults(const toml::table& convert_tbl, AppConfig& config)
    -> void {
  if (auto val = convert_tbl.get("date_check")->value<std::string>()) {
    config.command_defaults.convert_date_check_mode = ParseDateCheckMode(*val);
  }
  if (auto val = convert_tbl.get("save_processed_output")->value<bool>()) {
    config.command_defaults.convert_save_processed_output = val;
  }
  if (auto val = convert_tbl.get("validate_logic")->value<bool>()) {
    config.command_defaults.convert_validate_logic = val;
  }
  if (auto val = convert_tbl.get("validate_structure")->value<bool>()) {
    config.command_defaults.convert_validate_structure = val;
  }
}

auto ParseQueryDefaults(const toml::table& query_tbl, AppConfig& config)
    -> void {
  if (auto val = query_tbl.get("format")->value<std::string>()) {
    config.command_defaults.query_format = *val;
  }
}

auto ParseIngestDefaults(const toml::table& ingest_tbl, AppConfig& config)
    -> void {
  if (auto val = ingest_tbl.get("date_check")->value<std::string>()) {
    config.command_defaults.ingest_date_check_mode = ParseDateCheckMode(*val);
  }
  if (auto val = ingest_tbl.get("save_processed_output")->value<bool>()) {
    config.command_defaults.ingest_save_processed_output = val;
  }
}

auto ParseValidateLogicDefaults(const toml::table& validate_logic_tbl,
                                AppConfig& config) -> void {
  if (auto val = validate_logic_tbl.get("date_check")->value<std::string>()) {
    config.command_defaults.validate_logic_date_check_mode =
        ParseDateCheckMode(*val);
  }
}
}  // namespace

void ParseSystemSettings(const toml::table& tbl, const fs::path& exe_path,
                         AppConfig& config) {
  const toml::node_view<const toml::node> kSysNode = tbl["system"];

  // 如果 [system] 不存在，尝试 [general]
  if (!kSysNode) {
    if (tbl.contains("general")) {
      const toml::node_view<const toml::node> kGenNode = tbl["general"];
      // 复用逻辑或简单复制
      // 这里为了演示直接写
      if (auto val = kGenNode["error_log"].value<std::string>()) {
        config.error_log_path = exe_path / *val;
      } else {
        config.error_log_path = exe_path / "error.log";
      }
      if (auto val = kGenNode["export_root"].value<std::string>()) {
        config.export_path = *val;
      }
      config.default_save_processed_output =
          kGenNode["save_processed_output"].value_or(false);
      bool check = kGenNode["date_check_continuity"].value_or(false);
      config.default_date_check_mode =
          check ? DateCheckMode::kContinuity : DateCheckMode::kNone;
      return;
    }
  }

  if (kSysNode) {
    if (auto val = kSysNode["error_log"].value<std::string>()) {
      config.error_log_path = exe_path / *val;
    } else {
      config.error_log_path = exe_path / "error.log";
    }

    if (auto val = kSysNode["export_root"].value<std::string>()) {
      config.export_path = *val;
    }

    config.default_save_processed_output =
        kSysNode["save_processed_output"].value_or(false);

    bool check = kSysNode["date_check_continuity"].value_or(false);
    config.default_date_check_mode =
        check ? DateCheckMode::kContinuity : DateCheckMode::kNone;
  } else {
    config.error_log_path = exe_path / "error.log";
  }
}

void ParseCliDefaults(const toml::table& tbl, const fs::path& exe_path,
                      AppConfig& config) {
  if (const toml::table* defaults_tbl = tbl["defaults"].as_table()) {
    ParseGlobalDefaults(*defaults_tbl, exe_path, config);
  }

  const toml::table* commands_tbl = tbl["commands"].as_table();
  if (commands_tbl == nullptr) {
    return;
  }

  if (const toml::node* node = commands_tbl->get("export")) {
    if (const toml::table* sub_tbl = node->as_table()) {
      ParseExportDefaults(*sub_tbl, config);
    }
  }

  if (const toml::node* node = commands_tbl->get("convert")) {
    if (const toml::table* sub_tbl = node->as_table()) {
      ParseConvertDefaults(*sub_tbl, config);
    }
  }

  if (const toml::node* node = commands_tbl->get("query")) {
    if (const toml::table* sub_tbl = node->as_table()) {
      ParseQueryDefaults(*sub_tbl, config);
    }
  }

  if (const toml::node* node = commands_tbl->get("ingest")) {
    if (const toml::table* sub_tbl = node->as_table()) {
      ParseIngestDefaults(*sub_tbl, config);
    }
  }

  if (const toml::node* node = commands_tbl->get("validate-logic")) {
    if (const toml::table* sub_tbl = node->as_table()) {
      ParseValidateLogicDefaults(*sub_tbl, config);
    }
  }
}

void ParsePipelineSettings(const toml::table& tbl, const fs::path& config_dir,
                           AppConfig& config) {
  if (tbl.contains("converter")) {
    const auto& proc = tbl["converter"];

    if (auto val = proc["interval_config"].value<std::string>()) {
      config.pipeline.interval_processor_config_path = config_dir / *val;
    } else {
      throw std::runtime_error(
          "Missing 'interval_config' in [converter] section.");
    }

    if (const toml::table* parents = proc["initial_top_parents"].as_table()) {
      for (const auto& [key, val] : *parents) {
        if (auto path_str = val.value<std::string>()) {
          config.pipeline.initial_top_parents[fs::path(key.str())] =
              fs::path(*path_str);
        }
      }
    }
  } else {
    throw std::runtime_error("Missing [converter] configuration block.");
  }
}

void ParseReportPaths(const toml::table& tbl, const fs::path& config_dir,
                      AppConfig& config) {
  if (tbl.contains("reports")) {
    const auto& reports = tbl["reports"];

    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    auto load_paths = [&](const std::string& key, fs::path& day_path,
                          fs::path& month_path, fs::path& period_path,
                          fs::path& week_path, fs::path& year_path) -> void {
      if (reports[key].is_table()) {
        const auto& section = reports[key];
        if (auto val_str = section["day"].value<std::string>()) {
          day_path = config_dir / *val_str;
        }
        if (auto val_str = section["month"].value<std::string>()) {
          month_path = config_dir / *val_str;
        }
        if (auto val_str = section["period"].value<std::string>()) {
          period_path = config_dir / *val_str;
        }
        if (auto val_str = section["week"].value<std::string>()) {
          week_path = config_dir / *val_str;
        }
        if (auto val_str = section["year"].value<std::string>()) {
          year_path = config_dir / *val_str;
        }
      }
    };
    // NOLINTEND(bugprone-easily-swappable-parameters)

    load_paths("typst", config.reports.day_typ_config_path,
               config.reports.month_typ_config_path,
               config.reports.period_typ_config_path,
               config.reports.week_typ_config_path,
               config.reports.year_typ_config_path);

    load_paths("latex", config.reports.day_tex_config_path,
               config.reports.month_tex_config_path,
               config.reports.period_tex_config_path,
               config.reports.week_tex_config_path,
               config.reports.year_tex_config_path);

    load_paths("markdown", config.reports.day_md_config_path,
               config.reports.month_md_config_path,
               config.reports.period_md_config_path,
               config.reports.week_md_config_path,
               config.reports.year_md_config_path);

  } else {
    throw std::runtime_error("Missing [reports] configuration block.");
  }
}

}  // namespace ConfigParserUtils
