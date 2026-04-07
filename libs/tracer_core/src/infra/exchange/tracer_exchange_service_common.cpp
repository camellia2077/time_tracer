#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/loader/toml_loader_utils.hpp"

import tracer.core.infrastructure.exchange;
import tracer.core.infrastructure.config.loader.converter_config_loader;
import tracer.core.shared.canonical_text;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace infra_config = tracer::core::infrastructure::config;
namespace modtext = tracer::core::shared::canonical_text;

namespace {

#include "infra/exchange/detail/tracer_exchange_service_common_file_support_impl.inc"
#include "infra/exchange/detail/tracer_exchange_service_common_progress_impl.inc"

}  // namespace

// Package text normalization, staging directories, and extracted package I/O.
#include "infra/exchange/detail/tracer_exchange_service_common_package_support_impl.inc"

// Active converter config validation, backup, restore, and apply flows.
#include "infra/exchange/detail/tracer_exchange_service_common_converter_config_impl.inc"

// Progress observer bridge between tracer_exchange DTOs and file_crypto.
#include "infra/exchange/detail/tracer_exchange_service_common_crypto_options_impl.inc"

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
