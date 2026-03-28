mod bootstrap;
mod callbacks;
mod codec;
mod env_flags;
mod errors;
mod ffi;
mod invoke;
mod loader;
mod pipeline_client;
mod query_client;
mod report_client;
mod tracer_exchange_client;

// Facade module: keep public runtime API stable while delegating internals to focused submodules.
use std::ffi::c_void;

use libloading::Library;
use serde::Deserialize;

use crate::commands::handler::CommandContext;
use crate::error::AppError;

use self::loader::{load_runtime_symbols, resolve_core_dll_path};
pub use self::pipeline_client::PipelineClient;
pub use self::query_client::QueryClient;
pub use self::report_client::ReportClient;
pub use self::tracer_exchange_client::TracerExchangeClient;

#[derive(Debug, Deserialize, Clone)]
pub struct ResolvedCliPaths {
    pub db_path: String,
    pub runtime_output_root: String,
    pub converter_config_toml_path: String,
}

#[derive(Debug, Deserialize, Clone)]
pub struct CliDefaults {
    pub default_format: Option<String>,
}

#[derive(Debug, Deserialize, Clone)]
pub struct CliCommandDefaults {
    pub export_format: Option<String>,
    pub query_format: Option<String>,
    pub convert_date_check_mode: Option<String>,
    pub convert_save_processed_output: Option<bool>,
    pub convert_validate_logic: Option<bool>,
    pub convert_validate_structure: Option<bool>,
    pub ingest_date_check_mode: Option<String>,
    pub ingest_save_processed_output: Option<bool>,
    pub validate_logic_date_check_mode: Option<String>,
}

#[derive(Debug, Deserialize, Clone)]
pub struct CliConfig {
    pub default_save_processed_output: bool,
    pub default_date_check_mode: Option<String>,
    pub defaults: CliDefaults,
    pub command_defaults: CliCommandDefaults,
}

#[derive(Debug, Deserialize, Clone)]
pub struct TreeNode {
    pub name: String,
    pub path: Option<String>,
    pub duration_seconds: Option<i64>,
    #[serde(default)]
    pub children: Vec<TreeNode>,
}

#[derive(Debug, Deserialize, Clone)]
pub struct TreeResponse {
    pub ok: bool,
    pub found: bool,
    #[serde(default)]
    pub error_message: String,
    #[serde(default)]
    pub roots: Vec<String>,
    #[serde(default)]
    pub nodes: Vec<TreeNode>,
    #[serde(default)]
    pub error_code: String,
    #[serde(default)]
    pub error_category: String,
    #[serde(default)]
    pub hints: Vec<String>,
}

pub struct CoreApi {
    _lib: Library,
    symbols: ffi::RuntimeSymbols,
}

pub(crate) struct CoreRuntime {
    api: CoreApi,
    handle: *mut c_void,
}

pub struct RuntimeSession {
    runtime: CoreRuntime,
    cli_config: CliConfig,
    paths: ResolvedCliPaths,
}

impl Drop for CoreRuntime {
    fn drop(&mut self) {
        unsafe {
            (self.api.symbols.runtime_destroy)(self.handle);
        }
    }
}

impl CoreApi {
    pub fn load() -> Result<Self, AppError> {
        let dll_path = resolve_core_dll_path()?;
        let lib = unsafe { Library::new(&dll_path) }.map_err(|e| {
            AppError::DllCompatibility(format!(
                "Load core dll failed ({}): {e}",
                dll_path.display()
            ))
        })?;
        let symbols = load_runtime_symbols(&lib)?;
        Ok(Self { _lib: lib, symbols })
    }

    pub fn bootstrap(self, command_name: &str, ctx: &CommandContext) -> Result<RuntimeSession, AppError> {
        bootstrap::bootstrap(self, command_name, ctx)
    }
}

pub fn finalize_tracer_exchange_progress_line() {
    callbacks::finalize_crypto_progress_line();
}

impl RuntimeSession {
    pub fn pipeline(&self) -> PipelineClient<'_> {
        PipelineClient::new(&self.runtime)
    }

    pub fn query(&self) -> QueryClient<'_> {
        QueryClient::new(&self.runtime)
    }

    pub fn report(&self) -> ReportClient<'_> {
        ReportClient::new(&self.runtime)
    }

    pub fn exchange(&self) -> TracerExchangeClient<'_> {
        TracerExchangeClient::new(&self.runtime)
    }

    pub fn cli_config(&self) -> &CliConfig {
        &self.cli_config
    }

    pub fn paths(&self) -> &ResolvedCliPaths {
        &self.paths
    }
}

pub struct ResolvedCliContext {
    pub paths: ResolvedCliPaths,
    pub cli_config: CliConfig,
}
