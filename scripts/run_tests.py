import sys
import os
import subprocess
from sys import platform as _platform
import argparse

##############################################################################################################
# All tests that should be executed on each platform, all_tests will run on all

# c64_vice_tests   
# lldb_tests       
# dbgeng_tests     

mac_tests = ["remote_api_tests"]
unix_tests = ["remote_api_tests"]
windows_tests = []

all_tests = ["core_tests", 
             "readwrite_tests", 
             "script_tests", 
             "session_tests", 
             "ui_docking_tests", 
             "ui_tests"] 

##############################################################################################################

def run_tests(tests, rp):
    try:
        for i in tests:
            p = os.path.join(rp, i)
            args = [p]
            subprocess.check_call(args)

    except subprocess.CalledProcessError, e:
        print("Something went running when running " + p + " error: " + str(e))
        sys.exit(1)

    return

##############################################################################################################

def run_all(config):
    
    rp = os.path.join('t2-output', config)

    run_tests(all_tests, rp)

    if _platform == "win32":
        run_tests(windows_tests, rp)
    elif _platform == "darwin":
        run_tests(mac_tests, rp)
    else:
        run_tests(unix_tests, rp)

    return

##############################################################################################################

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run tests')
    parser.add_argument('--config', help='Which configuration to run the tests from')
    args = parser.parse_args()

    if (args.config == None):
        sys.exit(1)

    run_all(args.config)


