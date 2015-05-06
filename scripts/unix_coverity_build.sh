#!/bin/bash

rm -rfv cov-int
rm -rfv prodbg_cov.xz
tundra2 -c linux-gcc-release
cov-build --dir cov-int tundra2 -v linux-gcc-release
tar cavf prodbg_cov.xz cov-int
curl --form token=gBMZ9SVfbnqwMUGd7JkIvA --form email=daniel@collin.com --form file=@prodbg_cov.xz --form version="0.01" --form description="Linux GCC" https://scan.coverity.com/builds?project=emoon%2FProDBG
