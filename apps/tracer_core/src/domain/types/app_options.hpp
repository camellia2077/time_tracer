// domain/types/app_options.hpp
#ifndef DOMAIN_TYPES_APP_OPTIONS_H_
#define DOMAIN_TYPES_APP_OPTIONS_H_

#include <filesystem>

#include "domain/types/date_check_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions {
  fs::path input_path;
  bool run_all = false;
  bool convert = false;
  bool validate_structure = false;
  bool validate_logic = false;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;
};

#endif  // DOMAIN_TYPES_APP_OPTIONS_H_
