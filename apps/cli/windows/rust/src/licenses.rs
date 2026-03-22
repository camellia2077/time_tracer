pub const PROJECT_LICENSE_NAME: &str = "Apache License 2.0";
pub const PROJECT_LICENSE_URL: &str = "https://www.apache.org/licenses/LICENSE-2.0";

#[derive(Debug, Clone)]
pub struct ThirdPartyLicenseEntry {
    pub name: &'static str,
    pub component: &'static str,
    pub version: &'static str,
    pub license: &'static str,
    pub homepage: &'static str,
    pub license_url: &'static str,
    pub notes: &'static str,
}

pub const THIRD_PARTY_LICENSES: [ThirdPartyLicenseEntry; 10] = [
    ThirdPartyLicenseEntry {
        name: "nlohmann/json",
        component: "tracer_core",
        version: "3.12.0",
        license: "MIT",
        homepage: "https://github.com/nlohmann/json",
        license_url: "https://github.com/nlohmann/json/blob/develop/LICENSE.MIT",
        notes: "C++ JSON parser used by tracer_core and the C ABI transport boundary.",
    },
    ThirdPartyLicenseEntry {
        name: "Apache ECharts",
        component: "shared/assets",
        version: "5.x (vendored minified build)",
        license: "Apache-2.0",
        homepage: "https://echarts.apache.org/",
        license_url: "https://www.apache.org/licenses/LICENSE-2.0",
        notes: "Bundled chart rendering JS asset for report-chart export.",
    },
    ThirdPartyLicenseEntry {
        name: "clap",
        component: "rust_cli",
        version: "4.5",
        license: "Apache-2.0 OR MIT",
        homepage: "https://github.com/clap-rs/clap",
        license_url: "https://github.com/clap-rs/clap/blob/master/LICENSE",
        notes: "Rust CLI argument parsing.",
    },
    ThirdPartyLicenseEntry {
        name: "thiserror",
        component: "rust_cli",
        version: "2.0",
        license: "Apache-2.0 OR MIT",
        homepage: "https://github.com/dtolnay/thiserror",
        license_url: "https://github.com/dtolnay/thiserror/blob/master/LICENSE-MIT",
        notes: "Rust CLI error definitions.",
    },
    ThirdPartyLicenseEntry {
        name: "libloading",
        component: "rust_cli",
        version: "0.8",
        license: "ISC",
        homepage: "https://github.com/nagisa/rust_libloading",
        license_url: "https://github.com/nagisa/rust_libloading/blob/master/LICENSE",
        notes: "Rust dynamic library loading helper.",
    },
    ThirdPartyLicenseEntry {
        name: "serde",
        component: "rust_cli",
        version: "1.0",
        license: "Apache-2.0 OR MIT",
        homepage: "https://github.com/serde-rs/serde",
        license_url: "https://github.com/serde-rs/serde/blob/master/LICENSE-MIT",
        notes: "Rust serialization framework.",
    },
    ThirdPartyLicenseEntry {
        name: "serde_json",
        component: "rust_cli",
        version: "1.0",
        license: "Apache-2.0 OR MIT",
        homepage: "https://github.com/serde-rs/json",
        license_url: "https://github.com/serde-rs/json/blob/master/LICENSE-MIT",
        notes: "Rust JSON encoding/decoding.",
    },
    ThirdPartyLicenseEntry {
        name: "toml",
        component: "rust_cli",
        version: "0.8",
        license: "Apache-2.0 OR MIT",
        homepage: "https://github.com/toml-rs/toml",
        license_url: "https://github.com/toml-rs/toml/blob/main/crates/toml/LICENSE",
        notes: "Rust TOML parsing support.",
    },
    ThirdPartyLicenseEntry {
        name: "SQLite",
        component: "tracer_core runtime",
        version: "3.51.2",
        license: "Public Domain",
        homepage: "https://sqlite.org/",
        license_url: "https://sqlite.org/copyright.html",
        notes: "Embedded database engine used by tracer_core runtime.",
    },
    ThirdPartyLicenseEntry {
        name: "Zstandard (zstd)",
        component: "tracer_core runtime",
        version: "1.x",
        license: "BSD-3-Clause",
        homepage: "https://github.com/facebook/zstd",
        license_url: "https://github.com/facebook/zstd/blob/dev/LICENSE",
        notes: "Compression backend used for encrypted transfer payloads.",
    },
];

pub fn render_project_license() -> String {
    format!(
        "TimeTracer project license: {PROJECT_LICENSE_NAME}\nLicense URL: {PROJECT_LICENSE_URL}\nRun `licenses` to view third-party dependency license information."
    )
}

pub fn render_third_party_summary() -> String {
    let mut lines = vec![String::from("Third-party licenses (summary):")];
    for item in THIRD_PARTY_LICENSES {
        lines.push(format!(
            "  - {} [{}] {} ({})",
            item.name, item.component, item.license, item.version
        ));
    }
    lines.push("Run `licenses --full` for detailed third-party license fields.".to_string());
    lines.join("\n")
}

pub fn render_third_party_full() -> String {
    let mut lines = vec![String::from("Third-party licenses (full):")];
    for item in THIRD_PARTY_LICENSES {
        lines.push(String::new());
        lines.push(format!("- Name: {}", item.name));
        lines.push(format!("  Component: {}", item.component));
        lines.push(format!("  Version: {}", item.version));
        lines.push(format!("  License: {}", item.license));
        lines.push(format!("  Homepage: {}", item.homepage));
        lines.push(format!("  License URL: {}", item.license_url));
        lines.push(format!("  Notes: {}", item.notes));
    }
    lines.join("\n")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn project_license_text_mentions_apache() {
        let text = render_project_license();
        assert!(text.contains("Apache License 2.0"));
        assert!(text.contains("licenses"));
    }

    #[test]
    fn third_party_summary_contains_known_dependency() {
        let text = render_third_party_summary();
        assert!(text.contains("nlohmann/json"));
        assert!(text.contains("clap"));
    }
}
