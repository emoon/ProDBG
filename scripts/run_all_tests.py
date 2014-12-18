import os
import sys
import subprocess
import time
from sys import platform as _platform

root = ""

if _platform == "darwin":
	root = "t2-output/macosx_test-clang-debug-default"
elif _platform == "windows":
	root = "t2-output/win64-msvc-debug-default"
else:
	root = "t2-output/linux-gcc-debug-default"

for item in os.listdir(root):
	file_name = os.path.join(root, item)
	if os.path.isfile(file_name) and 'tests' in item:
		if item != "lldb_tests":
			ret = os.system(file_name)
			if ret != 0:
				sys.exit(1)

# Start ProDBG on Mac also

if _platform == "darwin":
	subprocess.Popen([os.path.join(root, 'ProDBG.app/Contents/MacOS/prodbg'), '', ''])
	time.sleep(5)
	os.system('osascript -e \"tell application \\\"ProDBG\\\" to quit\"')
	time.sleep(2)

