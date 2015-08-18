from psd_tools import PSDImage
import json
import sys
import os

def writeLayerImages(filenameIn, outFolderName):
  psd = PSDImage.load(filenameIn)

  if not os.path.exists(outFolderName):
    os.makedirs(outFolderName)
  
  for curLayer in psd.layers:
    img = curLayer.as_PIL()

    slash = "/"
    if os.name != 'posix':
      slash = "\\"

    img.save(outFolderName + slash + curLayer.name + ".png")

def main():
  psdFilename = sys.argv[1]
  outputFolder = sys.argv[2]
  
  writeLayerImages(psdFilename, outputFolder)

if __name__ == "__main__":
  if(len(sys.argv) != 3):
    print "Input Arguments: <input PSD File> <output Folder>"
  else:
    main()
  

