#/bin/bash
coveralls --exclude-pattern ".*as_debugger.*" --exclude-pattern ".*as_game.*" --exclude-pattern ".*Frameworks.*" --exclude-pattern ".*external.*" --exclude-pattern ".*glsl.*" --exclude-pattern ".*examples.*" -x cpp -x c
