use std::env;

fn main() {
	let target = env::var("TARGET").unwrap_or("".to_string());
    let tundra_dir = env::var("TUNDRA_OBJECTDIR").unwrap_or("".to_string());
    let libs = env::var("TUNDRA_STATIC_LIBS").unwrap_or("".to_string());

    let native_libs = libs.split(" ");

    println!("cargo:rustc-link-search=native={}", tundra_dir);

    for lib in native_libs {
        println!("cargo:rustc-link-lib=static={}", lib);
        println!("cargo:rerun-if-changed={}", lib);
    }

	if target.contains("darwin") {
		println!("cargo:rustc-flags=-l dylib=stdc++");
    	println!("cargo:rustc-flags=-l framework=Cocoa");
        println!("cargo:rustc-flags=-l framework=Metal");
        println!("cargo:rustc-flags=-l framework=OpenGL");
        println!("cargo:rustc-flags=-l framework=QuartzCore");
    } else if target.contains("windows") {
    } else {
		println!("cargo:rustc-flags=-l dylib=stdc++");
    	println!("cargo:rustc-flags=-l dylib=X11");
    	println!("cargo:rustc-flags=-l dylib=GL");
    	println!("cargo:rustc-flags=-l dylib=dl");
    }
}


