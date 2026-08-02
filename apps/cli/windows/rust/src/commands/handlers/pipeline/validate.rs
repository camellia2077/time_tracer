use crate::cli::{
    PipelineValidateAllArgs, PipelineValidateArgs, PipelineValidateBundleArgs,
    PipelineValidateCommand, PipelineValidateLogicArgs, PipelineValidateStructureArgs,
};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{build_validate_logic_request, build_validate_structure_request};
use super::{PipelineSessionPort, RuntimePipelineSessionPort};

pub struct ValidateHandler;

impl CommandHandler<PipelineValidateArgs> for ValidateHandler {
    fn handle(&self, args: PipelineValidateArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            PipelineValidateCommand::Structure(args) => {
                run_structure_with_port(args, ctx, &RuntimePipelineSessionPort)
            }
            PipelineValidateCommand::Logic(args) => {
                run_logic_with_port(args, ctx, &RuntimePipelineSessionPort)
            }
            PipelineValidateCommand::All(args) => {
                run_all_with_port(args, ctx, &RuntimePipelineSessionPort)
            }
            PipelineValidateCommand::Bundle(args) => {
                run_bundle_with_port(args, &RuntimePipelineSessionPort)
            }
        }
    }
}

fn run_bundle_with_port(
    args: PipelineValidateBundleArgs,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let date_check_mode = if args.no_date_check {
        "none"
    } else {
        match args.date_check.unwrap_or(crate::cli::DateCheckMode::None) {
            crate::cli::DateCheckMode::None => "none",
            crate::cli::DateCheckMode::Continuity => "continuity",
            crate::cli::DateCheckMode::Full => "full",
        }
    };
    port.validate_external_bundle(&args.txt_path, &args.config_path, date_check_mode)
}

pub(crate) fn run_structure_with_port(
    args: PipelineValidateStructureArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let request = build_validate_structure_request(&args.path);
    port.validate_structure("validate-structure", ctx, &request)
}

pub(crate) fn run_logic_with_port(
    args: PipelineValidateLogicArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let request = build_validate_logic_request(&args.path, args.date_check, args.no_date_check);
    port.validate_logic("validate-logic", ctx, &request)
}

pub(crate) fn run_all_with_port(
    args: PipelineValidateAllArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let structure_request = build_validate_structure_request(&args.path);
    port.validate_structure("validate-structure", ctx, &structure_request)?;
    let logic_request =
        build_validate_logic_request(&args.path, args.date_check, args.no_date_check);
    port.validate_logic("validate-logic", ctx, &logic_request)
}
