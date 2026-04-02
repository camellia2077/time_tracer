use thiserror::Error;

#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum AppExitCode {
    Success = 0,
    GenericError = 1,
    CommandNotFound = 2,
    InvalidArguments = 3,
    DatabaseError = 4,
    IoError = 5,
    LogicError = 6,
    ConfigError = 7,
    MemoryError = 8,
    UnknownError = 9,
    DllCompatibilityError = 10,
    ReportTargetNotFound = 11,
}

#[allow(dead_code)]
#[derive(Debug, Error)]
pub enum AppError {
    #[error("{message}")]
    Plain { message: String, code: AppExitCode },
    #[error("{0}")]
    CommandNotFound(String),
    #[error("Invalid arguments: {0}")]
    InvalidArguments(String),
    #[error("Database error: {0}")]
    Database(String),
    #[error("I/O error: {0}")]
    Io(String),
    #[error("Configuration error: {0}")]
    Config(String),
    #[error("DLL compatibility error: {0}")]
    DllCompatibility(String),
    #[error("Memory error: {0}")]
    Memory(String),
    #[error("Logic Error: {0}")]
    Logic(String),
    #[error("Unknown error: {0}")]
    Unknown(String),
    #[error("Generic error: {0}")]
    Generic(String),
}

impl AppError {
    pub fn exit_code(&self) -> AppExitCode {
        match self {
            AppError::Plain { code, .. } => *code,
            AppError::CommandNotFound(_) => AppExitCode::CommandNotFound,
            AppError::InvalidArguments(_) => AppExitCode::InvalidArguments,
            AppError::Database(_) => AppExitCode::DatabaseError,
            AppError::Io(_) => AppExitCode::IoError,
            AppError::Config(_) => AppExitCode::ConfigError,
            AppError::DllCompatibility(_) => AppExitCode::DllCompatibilityError,
            AppError::Memory(_) => AppExitCode::MemoryError,
            AppError::Logic(_) => AppExitCode::LogicError,
            AppError::Unknown(_) => AppExitCode::UnknownError,
            AppError::Generic(_) => AppExitCode::GenericError,
        }
    }

    pub fn render_for_stderr(&self) -> String {
        let rendered = self.to_string();
        if looks_like_contract_text(&rendered) {
            return rendered;
        }

        let (error_code, error_category, hint) = self.contract_triplet();
        if hint.is_empty() {
            return format!("[{error_code}] {rendered} (category={error_category})");
        }
        format!("[{error_code}] {rendered} (category={error_category}, hints={hint})")
    }

    fn contract_triplet(&self) -> (&'static str, &'static str, &'static str) {
        match self {
            AppError::Plain { code, .. } => match code {
                AppExitCode::CommandNotFound => (
                    "cli.command_not_found",
                    "cli",
                    "Run with --help to see available commands.",
                ),
                AppExitCode::InvalidArguments => (
                    "cli.invalid_arguments",
                    "cli",
                    "Run command --help to inspect required arguments.",
                ),
                AppExitCode::ConfigError => (
                    "runtime.config_error",
                    "config",
                    "Check runtime config under config/config.toml.",
                ),
                AppExitCode::DllCompatibilityError => (
                    "runtime.dll_compatibility_error",
                    "runtime",
                    "Check runtime DLL bundle near executable.",
                ),
                AppExitCode::LogicError => (
                    "runtime.logic_error",
                    "runtime",
                    "Inspect error_message for detailed reason.",
                ),
                AppExitCode::ReportTargetNotFound => (
                    "reporting.target.not_found",
                    "reporting",
                    "Check that the requested report target exists in the current database.",
                ),
                AppExitCode::DatabaseError => (
                    "runtime.database_error",
                    "runtime",
                    "Check database path and sqlite runtime dependencies.",
                ),
                AppExitCode::IoError => (
                    "runtime.io_error",
                    "runtime",
                    "Check filesystem path and permissions.",
                ),
                AppExitCode::MemoryError => (
                    "runtime.memory_error",
                    "runtime",
                    "Reduce input size or available memory pressure.",
                ),
                AppExitCode::UnknownError => (
                    "runtime.unknown_error",
                    "runtime",
                    "Inspect logs for full diagnostic details.",
                ),
                AppExitCode::Success | AppExitCode::GenericError => (
                    "runtime.generic_error",
                    "runtime",
                    "Inspect logs/output.log for failure details.",
                ),
            },
            AppError::CommandNotFound(_) => (
                "cli.command_not_found",
                "cli",
                "Run with --help to see available commands.",
            ),
            AppError::InvalidArguments(_) => (
                "cli.invalid_arguments",
                "cli",
                "Run command --help to inspect required arguments.",
            ),
            AppError::Database(_) => (
                "runtime.database_error",
                "runtime",
                "Check database path and sqlite runtime dependencies.",
            ),
            AppError::Io(_) => (
                "runtime.io_error",
                "runtime",
                "Check filesystem path and permissions.",
            ),
            AppError::Config(_) => (
                "runtime.config_error",
                "config",
                "Check runtime config under config/config.toml.",
            ),
            AppError::DllCompatibility(_) => (
                "runtime.dll_compatibility_error",
                "runtime",
                "Check runtime DLL bundle near executable.",
            ),
            AppError::Memory(_) => (
                "runtime.memory_error",
                "runtime",
                "Reduce input size or available memory pressure.",
            ),
            AppError::Logic(_) => (
                "runtime.logic_error",
                "runtime",
                "Inspect error_message for detailed reason.",
            ),
            AppError::Unknown(_) => (
                "runtime.unknown_error",
                "runtime",
                "Inspect logs for full diagnostic details.",
            ),
            AppError::Generic(_) => (
                "runtime.generic_error",
                "runtime",
                "Inspect logs/output.log for failure details.",
            ),
        }
    }
}

fn looks_like_contract_text(text: &str) -> bool {
    if text.trim().is_empty() {
        return false;
    }
    if !text.contains("category=") {
        return false;
    }
    text.starts_with('[') || text.contains(": [")
}
