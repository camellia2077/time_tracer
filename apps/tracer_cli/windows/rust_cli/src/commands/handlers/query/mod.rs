pub mod data;
pub mod tree;

use serde_json::Value;

use crate::cli::{QueryArgs, QueryCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CoreApi, TreeResponse};
use crate::error::AppError;

use self::data::DataHandler;
use self::tree::TreeHandler;

pub struct QueryHandler;

pub(crate) trait QuerySessionPort {
    fn data(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn tree(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TreeResponse, AppError>;
}

pub(crate) struct RuntimeQuerySessionPort;

impl QuerySessionPort for RuntimeQuerySessionPort {
    fn data(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.query().data(request)
    }

    fn tree(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TreeResponse, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.query().tree(request)
    }
}

impl CommandHandler<QueryArgs> for QueryHandler {
    fn handle(&self, args: QueryArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            QueryCommand::Data(args) => DataHandler.handle(args, ctx),
            QueryCommand::Tree(args) => TreeHandler.handle(args, ctx),
        }
    }
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::{QueryDataArgs, QueryPeriod, QueryTreeArgs, SuggestScoreMode};
    use crate::commands::testing::{RecordedQuerySession, default_context};
    use crate::core::runtime::TreeResponse;

    use super::QuerySessionPort;
    use super::data::run_data_with_port;
    use super::tree::run_tree_with_port;

    struct TestQueryPort<'a> {
        recorded: &'a RecordedQuerySession,
    }

    impl QuerySessionPort for TestQueryPort<'_> {
        fn data(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorded.record_data(command_name, request)
        }

        fn tree(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<TreeResponse, crate::error::AppError> {
            self.recorded.record_tree(command_name, request)
        }
    }

    #[test]
    fn query_data_maps_root_period_and_lookback_fields() {
        let recorded = RecordedQuerySession::new_success("{}");
        let port = TestQueryPort {
            recorded: &recorded,
        };

        run_data_with_port(
            QueryDataArgs {
                action: "activity-suggest".to_string(),
                data_output: Some(crate::cli::DataOutputMode::Json),
                year: None,
                month: None,
                from: None,
                to: None,
                remark: None,
                day_remark: None,
                root: Some("study".to_string()),
                overnight: false,
                exercise: None,
                status: None,
                numbers: None,
                top: Some(5),
                lookback_days: Some(10),
                activity_prefix: Some("stu".to_string()),
                score_mode: Some(SuggestScoreMode::Duration),
                period: Some(QueryPeriod::Day),
                period_arg: Some("20260103".to_string()),
                level: Some(2),
                reverse: true,
            },
            &default_context(),
            &port,
        )
        .expect("query data should succeed");

        assert_eq!(recorded.command_names(), vec!["query".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["action"], "activity-suggest");
        assert_eq!(request["output_mode"], "json");
        assert_eq!(request["root"], "study");
        assert_eq!(request["top_n"], 5);
        assert_eq!(request["lookback_days"], 10);
        assert_eq!(request["activity_prefix"], "stu");
        assert_eq!(request["activity_score_by_duration"], true);
        assert_eq!(request["tree_period"], "day");
        assert_eq!(request["tree_period_argument"], "20260103");
        assert_eq!(request["tree_max_depth"], 2);
        assert_eq!(request["reverse"], true);
    }

    #[test]
    fn query_tree_roots_request_uses_tree_bootstrap_token() {
        let recorded = RecordedQuerySession::new_success("{}").with_tree_response(TreeResponse {
            ok: true,
            found: true,
            error_message: String::new(),
            roots: vec!["study".to_string()],
            nodes: Vec::new(),
            error_code: String::new(),
            error_category: String::new(),
            hints: Vec::new(),
        });
        let port = TestQueryPort {
            recorded: &recorded,
        };

        run_tree_with_port(
            QueryTreeArgs {
                root: None,
                level: None,
                roots: true,
            },
            &default_context(),
            &port,
        )
        .expect("tree roots should succeed");

        assert_eq!(recorded.command_names(), vec!["tree".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["list_roots"], true);
        assert_eq!(request["root_pattern"], "");
        assert_eq!(request["max_depth"], -1);
    }

    #[test]
    fn query_tree_root_and_level_request_are_forwarded() {
        let recorded = RecordedQuerySession::new_success("{}").with_tree_response(TreeResponse {
            ok: true,
            found: true,
            error_message: String::new(),
            roots: Vec::new(),
            nodes: Vec::new(),
            error_code: String::new(),
            error_category: String::new(),
            hints: Vec::new(),
        });
        let port = TestQueryPort {
            recorded: &recorded,
        };

        run_tree_with_port(
            QueryTreeArgs {
                root: Some("study".to_string()),
                level: Some(1),
                roots: false,
            },
            &default_context(),
            &port,
        )
        .expect("tree path query should succeed");

        let request = recorded.requests().remove(0);
        assert_eq!(request["list_roots"], false);
        assert_eq!(request["root_pattern"], "study");
        assert_eq!(request["max_depth"], 1);
    }
}
