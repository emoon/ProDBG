extern crate gcc;

fn main() {
    gcc::compile_library("libpoll.a", &["poll.c"]);
}
