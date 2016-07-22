rust-bindgen:
	git clone https://github.com/crabtw/rust-bindgen

rust-bindgen/target/release/bindgen: rust-bindgen
	cd rust-bindgen && git checkout 6e33fbe74de0c38c68658cc51c6d8fe584691367
	cd rust-bindgen && cargo build --release

src/ffi_bgfx.rs: rust-bindgen/target/release/bindgen bgfx/include/bgfx/c99/bgfx.h bgfx/include/bgfx/c99/bgfxplatform.h
	rust-bindgen/target/release/bindgen \
		-match bgfx.h \
		-I bgfx/include \
		-I bx/include \
		-o src/ffi_bgfx.rs \
		bgfx/include/bgfx/c99/bgfx.h

src/ffi_bgfxplatform.rs: rust-bindgen/target/release/bindgen bgfx/include/bgfx/c99/bgfx.h bgfx/include/bgfx/c99/bgfxplatform.h
	rust-bindgen/target/release/bindgen \
		-match bgfxplatform.h \
		-I bgfx/include \
		-I bx/include \
		-include bgfx/c99/bgfx.h \
		-o src/ffi_bgfxplatform.rs \
		bgfx/include/bgfx/c99/bgfxplatform.h
