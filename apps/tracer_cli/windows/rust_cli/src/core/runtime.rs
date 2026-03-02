mod bootstrap;
mod callbacks;
mod codec;
mod env_flags;
mod errors;
mod ffi;
mod invoke;
mod loader;

// Facade module: keep public runtime API stable while delegating internals to focused submodules.
use std::ffi::c_void;

use libloading::Library;
use serde::Deserialize;
use serde_json::Value;

use crate::commands::handler::CommandContext;
use crate::error::AppError;

use self::loader::{load_runtime_symbols, resolve_core_dll_path};

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

#[derive(Debug, Deserialize)]
pub struct TreeNode {
    pub name: String,
    pub path: Option<String>,
    pub duration_seconds: Option<i64>,
    #[serde(default)]
    pub children: Vec<TreeNode>,
}

#[derive(Debug, Deserialize)]
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

pub struct CoreRuntime<'a> {
    api: &'a CoreApi,
    handle: *mut c_void,
}

impl Drop for CoreRuntime<'_> {
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

    pub fn bootstrap(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<(CoreRuntime<'_>, CliConfig), AppError> {
        bootstrap::bootstrap(self, command_name, ctx)
    }
}

pub fn finalize_crypto_progress_line() {
    callbacks::finalize_crypto_progress_line();
}

impl CoreRuntime<'_> {
    pub fn run_query(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_query(self, request)
    }

    pub fn run_report(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_report(self, request)
    }

    pub fn run_report_batch(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_report_batch(self, request)
    }

    pub fn run_export(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_export(self, request)
    }

    pub fn run_convert(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_convert(self, request)
    }

    pub fn run_import(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_import(self, request)
    }

    pub fn run_ingest(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_ingest(self, request)
    }

    pub fn run_validate_structure(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_validate_structure(self, request)
    }

    pub fn run_validate_logic(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_validate_logic(self, request)
    }

    pub fn run_tree(&self, request: &Value) -> Result<TreeResponse, AppError> {
        invoke::run_tree(self, request)
    }

    pub fn run_crypto_encrypt(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_crypto_encrypt(self, request)
    }

    pub fn run_crypto_decrypt(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_crypto_decrypt(self, request)
    }

    pub fn run_crypto_inspect(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_crypto_inspect(self, request)
    }
}

pub struct ResolvedCliContext {
    pub paths: ResolvedCliPaths,
    pub cli_config: CliConfig,
}
