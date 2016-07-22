bgfx-rs [![travis-ci status](https://travis-ci.org/rhoot/bgfx-rs.svg?branch=master)](https://travis-ci.org/rhoot/bgfx-rs) [![appveyor status](https://ci.appveyor.com/api/projects/status/github/rhoot/bgfx-rs?branch=master&svg=true)](https://ci.appveyor.com/project/rhoot/bgfx-rs/branch/master)
=======

Rust wrapper around [bgfx], providing a clean, safe API for rust applications.

*Please read the [crate documentation][docs] for build requirements and crate
limitations before using.*

Documentation
-------------

[API Documentation][docs]

### Examples

To run the examples, invoke them through cargo:

```
cargo run --example 00-helloworld
cargo run --example 01-cubes
```

**OSX Note:** There is currently no really clean way to exit the examples in
OSX, and closing the window may in fact cause a crash. This is due to
limitations in [glutin][glutin] (specifically [#468] and [#520]). This only
effects the examples, and not the crate itself. The best way of closing them
is to simply `Ctrl-C` in the console.

License
-------
Copyright (c) 2015-2016, Johan Sköld

Permission to use, copy, modify, and/or distribute this software for any  
purpose with or without fee is hereby granted, provided that the above  
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES  
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF  
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES  
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN  
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


[#468]:   https://github.com/tomaka/glutin/issues/468   "tomaka/glutin #468"
[#520]:   https://github.com/tomaka/glutin/issues/520   "tomaka/glutin #520"
[bgfx]:   https://github.com/bkaradzic/bgfx             "bgfx"
[docs]:   https://rhoot.github.io/bgfx-rs/bgfx/         "Bindings documentation"
[glutin]: https://github.com/tomaka/glutin              "glutin"