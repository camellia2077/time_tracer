use serde_json::json;

use crate::cli::QueryTreeArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::TreeNode;
use crate::error::AppError;

use super::{QuerySessionPort, RuntimeQuerySessionPort};

pub struct TreeHandler;

impl CommandHandler<QueryTreeArgs> for TreeHandler {
    fn handle(&self, args: QueryTreeArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_tree_with_port(args, ctx, &RuntimeQuerySessionPort)
    }
}

pub(crate) fn run_tree_with_port(
    args: QueryTreeArgs,
    ctx: &CommandContext,
    port: &dyn QuerySessionPort,
) -> Result<(), AppError> {
    if args.roots {
        let response = port.tree(
            "tree",
            ctx,
            &json!({
                "list_roots": true,
                "root_pattern": "",
                "max_depth": -1
            }),
        )?;
        for root in response.roots {
            println!("{root}");
        }
        return Ok(());
    }

    let response = port.tree(
        "tree",
        ctx,
        &json!({
            "list_roots": false,
            "root_pattern": args.root.clone().unwrap_or_default(),
            "max_depth": args.level.unwrap_or(-1),
        }),
    )?;

    if !response.found {
        if let Some(root) = args.root {
            eprintln!("No project found matching path: {root}.");
        } else {
            eprintln!("No project entries were found.");
        }
        return Ok(());
    }

    for node in response.nodes {
        print_node(&node, 0);
    }
    Ok(())
}

fn print_node(node: &TreeNode, depth: usize) {
    let indent = "  ".repeat(depth);
    let mut line = format!("{indent}{}", node.name);
    if let Some(path) = node.path.as_ref() {
        line.push_str(&format!(" [{path}]"));
    }
    if let Some(seconds) = node.duration_seconds {
        line.push_str(&format!(" ({seconds}s)"));
    }
    println!("{line}");
    for child in &node.children {
        print_node(child, depth + 1);
    }
}
