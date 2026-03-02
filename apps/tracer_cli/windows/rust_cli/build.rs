#[cfg(target_os = "windows")]
fn main() {
    use std::env;
    use std::path::PathBuf;

    println!("cargo:rerun-if-env-changed=TT_WINDOWS_CLI_ICON_ICO");

    let icon_path = match env::var_os("TT_WINDOWS_CLI_ICON_ICO") {
        Some(value) => PathBuf::from(value),
        None => {
            println!(
                "cargo:warning=TT_WINDOWS_CLI_ICON_ICO is not set; building without embedded icon."
            );
            return;
        }
    };

    if !icon_path.exists() {
        panic!(
            "TT_WINDOWS_CLI_ICON_ICO points to a missing file: {}",
            icon_path.display()
        );
    }

    println!("cargo:rerun-if-changed={}", icon_path.display());

    let mut res = winres::WindowsResource::new();
    res.set_icon(icon_path.to_string_lossy().as_ref());

    if let Err(err) = res.compile() {
        panic!("failed to compile Windows resources: {err}");
    }
}

#[cfg(not(target_os = "windows"))]
fn main() {}
