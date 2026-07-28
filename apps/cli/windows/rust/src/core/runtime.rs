mod bootstrap;
mod alias_hierarchy_client;
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
mod txt_client;

// Facade module: keep public runtime API stable while delegating internals to focused submodules.
use std::ffi::c_void;
use std::fs;
use std::path::{Path, PathBuf};

use libloading::Library;
use serde::Deserialize;
use serde_json::json;

use crate::commands::handler::CommandContext;
use crate::error::AppError;

use self::loader::{load_runtime_symbols, resolve_core_dll_path};
use self::bootstrap::create_runtime;
use self::callbacks::configure_callbacks;
pub use self::pipeline_client::PipelineClient;
pub use self::query_client::QueryClient;
pub use self::report_client::ReportClient;
pub use self::tracer_exchange_client::TracerExchangeClient;
pub use self::alias_hierarchy_client::{
    AliasCanonicalReplacement as AliasHierarchyCanonicalReplacement,
    AliasHierarchyClient, AliasHierarchyOperationOutput, AliasKeyReplacement,
};
pub use self::txt_client::{
    TxtCanonicalReplaceOutput, TxtClient, TxtReplaceOutput, TxtResolveOutput,
};

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

    pub fn bootstrap(
        self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<RuntimeSession, AppError> {
        bootstrap::bootstrap(self, command_name, ctx)
    }

    pub(crate) fn validate_external_bundle(
        self,
        txt_path: &Path,
        config_root: &Path,
        date_check_mode: &str,
    ) -> Result<(), AppError> {
        validate_external_toml_files(config_root)?;
        let converter_config = config_root.join("aliases").join("_system.toml");
        if !converter_config.is_file() {
            return Err(AppError::Config(format!(
                "External config is missing aliases/_system.toml: {}",
                converter_config.display()
            )));
        }
        if !txt_path.is_dir() {
            return Err(AppError::InvalidArguments(format!(
                "TXT path is not a directory: {}",
                txt_path.display()
            )));
        }

        let output_root = std::env::temp_dir().join(format!(
            "time_tracer_cli_validate_{}",
            std::process::id()
        ));
        fs::create_dir_all(&output_root).map_err(|error| {
            AppError::Io(format!(
                "Create validation workspace failed ({}): {error}",
                output_root.display()
            ))
        })?;
        configure_callbacks(&self);
        let paths = ResolvedCliPaths {
            db_path: String::new(),
            runtime_output_root: output_root.to_string_lossy().into_owned(),
            converter_config_toml_path: converter_config.to_string_lossy().into_owned(),
        };
        let runtime = create_runtime(self, &paths)?;
        let result = (|| {
            let input_path = txt_path.to_string_lossy();
            PipelineClient::new(&runtime).validate_structure(&json!({
                "input_path": input_path.as_ref(),
            }))?;
            PipelineClient::new(&runtime).validate_logic(&json!({
                "input_path": input_path.as_ref(),
                "date_check_mode": date_check_mode,
            }))
        })();
        drop(runtime);
        let _ = fs::remove_dir_all(&output_root);
        result
    }
}

fn validate_external_toml_files(config_root: &Path) -> Result<(), AppError> {
    if !config_root.is_dir() {
        return Err(AppError::InvalidArguments(format!(
            "Config path is not a directory: {}",
            config_root.display()
        )));
    }
    let main_config = config_root.join("config.toml");
    if !main_config.is_file() {
        return Err(AppError::Config(format!(
            "External config is missing config.toml: {}",
            main_config.display()
        )));
    }
    let mut files = Vec::<PathBuf>::new();
    collect_toml_files(config_root, &mut files).map_err(|error| {
        AppError::Io(format!(
            "Scan external config failed ({}): {error}",
            config_root.display()
        ))
    })?;
    if files.is_empty() {
        return Err(AppError::Config(format!(
            "External config contains no TOML files: {}",
            config_root.display()
        )));
    }
    for path in files {
        let content = fs::read_to_string(&path).map_err(|error| {
            AppError::Io(format!("Read TOML failed ({}): {error}", path.display()))
        })?;
        toml::from_str::<toml::Value>(&content).map_err(|error| {
            AppError::Config(format!("Invalid TOML ({}): {error}", path.display()))
        })?;
    }
    Ok(())
}

fn collect_toml_files(root: &Path, files: &mut Vec<PathBuf>) -> std::io::Result<()> {
    for entry in fs::read_dir(root)? {
        let entry = entry?;
        let path = entry.path();
        if path.is_dir() {
            collect_toml_files(&path, files)?;
        } else if path.extension().and_then(|value| value.to_str()) == Some("toml") {
            files.push(path);
        }
    }
    Ok(())
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

    pub fn txt(&self) -> TxtClient<'_> {
        TxtClient::new(&self.runtime)
    }

    pub fn alias_hierarchy(&self) -> AliasHierarchyClient<'_> {
        AliasHierarchyClient::new(&self.runtime)
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
