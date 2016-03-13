#!/bin/bash 
bin/macosx/tundra/tundra2 macosx-clang-release
cargo build --release
cargo build --manifest-path=src/plugins/bitmap_memory/Cargo.toml --release
cargo build --manifest-path=src/prodbg/tests/rust_api_test/Cargo.toml --release

