mod append;
mod format;
mod view;

#[cfg(test)]
mod tests;

use serde_json::Value;

use crate::cli::{TxtAppendEventArgs, TxtArgs, TxtCommand, TxtViewDayArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CoreApi, TxtReplaceOutput, TxtResolveOutput};
use crate::error::AppError;

pub struct TxtHandler;

pub(crate) trait TxtSessionPort {
    fn resolve_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError>;
    fn replace_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtReplaceOutput, AppError>;
}

pub(crate) struct RuntimeTxtSessionPort;

impl TxtSessionPort for RuntimeTxtSessionPort {
    fn resolve_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.txt().resolve_day_block(request)
    }

    fn replace_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtReplaceOutput, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.txt().replace_day_block(request)
    }
}

impl CommandHandler<TxtArgs> for TxtHandler {
    fn handle(&self, args: TxtArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            TxtCommand::ViewDay(args) => ViewDayHandler.handle(args, ctx),
            TxtCommand::AppendEvent(args) => AppendEventHandler.handle(args, ctx),
        }
    }
}

struct ViewDayHandler;
struct AppendEventHandler;

impl CommandHandler<TxtViewDayArgs> for ViewDayHandler {
    fn handle(&self, args: TxtViewDayArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let body = run_view_day_with_port(args, ctx, &RuntimeTxtSessionPort)?;
        print!("{body}");
        Ok(())
    }
}

impl CommandHandler<TxtAppendEventArgs> for AppendEventHandler {
    fn handle(&self, args: TxtAppendEventArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let output = run_append_event_with_port(args, ctx, &RuntimeTxtSessionPort)?;
        print!("{output}");
        Ok(())
    }
}

pub(crate) use append::run as run_append_event_with_port;
#[cfg(test)]
pub(crate) use format::{
    append_event_to_day_body, build_authored_event_line, infer_selected_month_from_path,
};
pub(crate) use view::run as run_view_day_with_port;
