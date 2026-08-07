pub mod export;
pub mod import;
pub mod inspect;
pub mod support;
#[cfg(test)]
mod tests;

use serde_json::Value;

use crate::cli::{ExchangeArgs, ExchangeCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi};
use crate::error::AppError;

use self::export::ExportHandler;
use self::import::ImportHandler;
use self::inspect::InspectHandler;

pub struct ExchangeHandler;

pub(crate) trait ExchangeSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError>;
    fn export_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn import_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn inspect_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
}
pub(crate) trait ExchangePromptPort {
    fn prompt_export_passphrase(&self) -> Result<String, AppError>;
    fn prompt_package_passphrase(&self) -> Result<String, AppError>;
}

pub(crate) struct RuntimeExchangeSessionPort;

impl ExchangeSessionPort for RuntimeExchangeSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        Ok(session.cli_config().clone())
    }

    fn export_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().export_package(request)
    }

    fn import_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().import_package(request)
    }

    fn inspect_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().inspect_package(request)
    }
}

pub(crate) struct InteractiveExchangePromptPort;

impl ExchangePromptPort for InteractiveExchangePromptPort {
    fn prompt_export_passphrase(&self) -> Result<String, AppError> {
        support::prompt_passphrase_for_export()
    }

    fn prompt_package_passphrase(&self) -> Result<String, AppError> {
        support::prompt_passphrase_for_package()
    }
}

struct ExchangeProgressLineGuard;

impl Drop for ExchangeProgressLineGuard {
    fn drop(&mut self) {
        crate::core::runtime::finalize_tracer_exchange_progress_line();
    }
}

impl CommandHandler<ExchangeArgs> for ExchangeHandler {
    fn handle(&self, args: ExchangeArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let _progress_guard = ExchangeProgressLineGuard;
        match args.command {
            ExchangeCommand::Export(args) => ExportHandler.handle(args, ctx),
            ExchangeCommand::Import(args) => ImportHandler.handle(args, ctx),
            ExchangeCommand::Inspect(args) => InspectHandler.handle(args, ctx),
        }
    }
}
