use std::env;

fn main() {
    let tundra_dir = env::var("TUNDRA_OBJECTDIR").unwrap_or("".to_string());
    let libs = env::var("TUNDRA_STATIC_LIBS").unwrap_or("".to_string());

    let native_libs = libs.split(" ");

    println!("cargo:rustc-link-search=native={}", tundra_dir);

    for lib in native_libs {
        println!("cargo:rustc-link-lib=static={}", lib);
        println!("cargo:rerun-if-changed={}", lib);
    }

    println!("cargo:rustc-flags=-l dylib=stdc++");
    println!("cargo:rustc-flags=-l framework=Cocoa");
}


