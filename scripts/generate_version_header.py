import argparse
import time
import os
import subprocess

#######################################################################################################################

def getSha(repo):
    """
    Grabs the current SHA-1 hash of the given directory's git HEAD-revision.
    The output of this is equivalent to calling git rev-parse HEAD.

    Be aware that a missing git repository will make this return an error message, 
    which is not a valid hash.
    """

    try:
        sha = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo)
        return sha.decode('ascii').strip()[:10]
    except:
        return "Unknown"

#######################################################################################################################

def main():
	
	parser = argparse.ArgumentParser()
	parser.add_argument("--file", help="Target file for the header", default="src/prodbg/main/prodbg_version.h")
	parser.add_argument("--version", help="Select the version", default="0.01")
	args = parser.parse_args()

        with open(args.file, "w") as out_file:
            version_string = "ProDBG " + args.version + " " + time.strftime("%c") + " (Git: " + getSha(".") + ")"

            out_file.write("#pragma once\n\n")
            out_file.write("// This file may or may not be auto generated (yes, really)\n\n")
            out_file.write("#ifdef PRODBG_WIN32\n")
            out_file.write('#define PRODBG_VERSION L\"' + version_string + '\"\n')
            out_file.write("#elif PRODBG_MAC\n");
            out_file.write('#define PRODBG_VERSION @\"' + version_string + '\"\n')
            out_file.write("#else\n");
            out_file.write('#define PRODBG_VERSION \"' + version_string + '\"\n')
            out_file.write("#endif\n");

#######################################################################################################################

if __name__ == "__main__":
   main()





