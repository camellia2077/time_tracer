use std::fs;
use std::path::PathBuf;

use serde_json::json;

use crate::cli::AliasTreeArgs;
use crate::commands::handler::CommandContext;
use crate::core::runtime::{ActivityHierarchyNodeKind, ActivityHierarchyTreeNode, CoreApi};
use crate::error::AppError;

pub(super) fn render_tree(args: AliasTreeArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let toml_content = fs::read_to_string(&path).map_err(|error| {
        AppError::Io(format!(
            "Read canonical TOML {} failed: {error}",
            path.display()
        ))
    })?;
    let core = CoreApi::load()?;
    let runtime = core.bootstrap("alias-tree", &ctx.without_output())?;
    let hierarchy = runtime.activity_hierarchy().describe(&json!({
        "action": "describe_activity_hierarchy",
        "toml_content": toml_content,
    }))?;
    print!(
        "{}",
        render_activity_hierarchy_text(&hierarchy.parent, &hierarchy.nodes, args.show_aliases)
    );
    Ok(())
}

fn render_activity_hierarchy_text(
    parent: &str,
    nodes: &[ActivityHierarchyTreeNode],
    show_aliases: bool,
) -> String {
    let mut output = String::new();
    output.push_str(parent);
    output.push('\n');
    let mut sorted_nodes = nodes.to_vec();
    sorted_nodes.sort_by(|left, right| left.canonical_key.cmp(&right.canonical_key));
    for (index, node) in sorted_nodes.iter().enumerate() {
        render_activity_hierarchy_node(
            node,
            String::new(),
            index + 1 == sorted_nodes.len(),
            false,
            show_aliases,
            &mut output,
        );
    }
    output
}

fn render_activity_hierarchy_node(
    node: &ActivityHierarchyTreeNode,
    prefix: String,
    is_last: bool,
    is_root: bool,
    show_aliases: bool,
    output: &mut String,
) {
    if is_root {
        output.push_str(&node.canonical_key);
    } else {
        output.push_str(&prefix);
        output.push_str(if is_last { "└── " } else { "├── " });
        output.push_str(&node.canonical_key);
    }

    if show_aliases && !node.aliases.is_empty() {
        output.push_str(" — ");
        output.push_str(match node.kind {
            ActivityHierarchyNodeKind::Group => "group_aliases: ",
            ActivityHierarchyNodeKind::Leaf => "aliases: ",
        });
        let mut aliases = node.aliases.clone();
        aliases.sort();
        output.push_str(&aliases.join(", "));
    }
    output.push('\n');

    let child_prefix = if is_root {
        String::new()
    } else {
        format!("{}{}", prefix, if is_last { "    " } else { "│   " })
    };
    let mut children = node.children.clone();
    children.sort_by(|left, right| left.canonical_key.cmp(&right.canonical_key));
    for (index, child) in children.iter().enumerate() {
        render_activity_hierarchy_node(
            child,
            child_prefix.clone(),
            index + 1 == children.len(),
            false,
            show_aliases,
            output,
        );
    }
}
