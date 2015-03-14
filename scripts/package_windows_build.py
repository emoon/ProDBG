import zipfile
import argparse
import tempfile
import shutil
import os

#######################################################################################################################

def copyDir(targetDir, path, filters):
		targetDir = os.path.join(targetDir, path)
		if filters != None:
			print("Copying " + path + " to " + targetDir + " with ignore filters " + str(filters))
			# shutil.rmtree(targetDir)
			shutil.copytree(path, targetDir, ignore=shutil.ignore_patterns(*filters))
		else:
			print("Copying " + path + " to " + targetDir)
			# shutil.rmtree(targetDir)
			shutil.copytree(path, targetDir)

#######################################################################################################################

def copyFile(target, path):
	shutil.copy2(path, target)

#######################################################################################################################

def getConfigPath(config):
	if config == "Release":
		return "t2-output\\win64-msvc-release-default"
	elif config == "Debug":
		return "t2-output\\win64-msvc-debug-default"
	else:
		return None

#######################################################################################################################

def zipDirWalk(path, zip):
	for root, dirs, files in os.walk(path):
		for file in files:
			out_path = os.path.join(str(root.replace(path, "")), file)
			print("out_path " + out_path)
			zip.write(os.path.join(root, file), out_path) 

#######################################################################################################################

def zipBuild(targetFile, sourceDir):
	print("Compressing build to zip..")
	zipf = zipfile.ZipFile(targetFile, 'w', zipfile.ZIP_DEFLATED, True)
	zipDirWalk(sourceDir, zipf)
	zipf.close()
	print("Compression done.")

#######################################################################################################################

def main():
	
	parser = argparse.ArgumentParser()
	parser.add_argument("--file", help="Target file for the compressed Build")
	parser.add_argument("--config", help="Select the configuration", default="Release")
	args = parser.parse_args()

	print(args.file)
	print(args.config)
	print(tempfile.gettempdir())

	prodbg_path = os.path.join(tempfile.gettempdir(), "prodbg")

	if (os.path.isdir(prodbg_path)):
		shutil.rmtree(prodbg_path)
	
	os.makedirs(prodbg_path)

	config_path = getConfigPath(args.config)

	# copy all the data to tempory directy

	copyDir(prodbg_path, os.path.join(config_path), ('*.pdb', '*.obj', '*.ilk', '*.lib', '*.exp', '*.exe')) 
	copyDir(prodbg_path, "temp", None) 
	copyDir(prodbg_path, "data", None) 
	copyFile(prodbg_path, os.path.join(config_path, "prodbg.exe"))

	# Compress to zip file

	zipBuild(args.file, prodbg_path)

if __name__ == "__main__":
   main()





