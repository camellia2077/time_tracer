mod about;
mod activity;
mod alias;
mod chart;
mod doctor;
mod exchange;
mod licenses;
mod pipeline;
mod query;
mod report;
mod root;
mod system;
mod txt;

pub use about::{AboutArgs, AboutCommand};
pub use activity::{ActivityArgs, ActivityCommand, ActivityMergeArgs};
pub use alias::{
    AliasAddArgs, AliasArgs, AliasCommand, AliasCreateArgs, AliasFileArgs, AliasGroupArgs,
    AliasMoveArgs, AliasMoveConfigArgs, AliasRenameGroupArgs, AliasRenameParentArgs, AliasTreeArgs,
};
pub use chart::{ChartArgs, ChartTheme, ChartType};
pub use doctor::DoctorArgs;
pub use exchange::{
    ExchangeArgs, ExchangeCommand, ExchangeExportArgs, ExchangeImportArgs, ExchangeInspectArgs,
    ExchangeUnpackArgs, SecurityLevel,
};
pub use licenses::LicensesArgs;
pub use pipeline::{
    DateCheckMode, PipelineArgs, PipelineCommand, PipelineConvertArgs, PipelineImportArgs,
    PipelineIngestArgs, PipelineValidateAllArgs, PipelineValidateArgs, PipelineValidateBundleArgs,
    PipelineValidateCommand, PipelineValidateLogicArgs, PipelineValidateStructureArgs,
};
pub use query::{
    DataOutputMode, QueryArgs, QueryCommand, QueryDataArgs, QueryPeriod, QueryTreeArgs,
    SuggestScoreMode,
};
pub use report::{
    ReportArgs, ReportCommand, ReportExportArgs, ReportExportPeriod, ReportFormat,
    ReportRenderArgs, ReportRenderPeriod,
};
pub use system::{SystemArgs, SystemCommand};
pub use txt::{TxtAppendEventArgs, TxtArgs, TxtCommand, TxtViewDayArgs};

pub use root::{Cli, Command};

#[cfg(test)]
mod tests;
