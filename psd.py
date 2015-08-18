from psd_tools import PSDImage
import json
import sys

def writeJSON(filenameIn, outFilename):
  psd = PSDImage.load(filenameIn)
  canvasWidth = psd.header.width 
  canvasHeight = psd.header.height
  
  header = {}
  header["canvasWidth"] = canvasWidth
  header["canvasHeight"] = canvasHeight
  
  regions = []

  i = 0
  for curLayer in psd.layers:
    regionData = {}
    regionData["width"] = curLayer.bbox.width
    regionData["height"] = curLayer.bbox.height
    regionData["x"] = curLayer.bbox.x1
    regionData["y"] = curLayer.bbox.y1
    regionData["name"] = curLayer.name
  
    regions.append(regionData)
    i += 1

    print "Writing out region: %s" %curLayer.name

  finalData = {}
  finalData["header"] = header
  finalData["regions"] = regions

  writeStr = json.dumps(finalData, ensure_ascii=True, sort_keys=True)
  text_file = open(outFilename, "w")
  text_file.write(writeStr)
  text_file.close()

def main():
  psdFilename = sys.argv[1]
  jsonFilename = sys.argv[2]
  
  writeJSON(psdFilename, jsonFilename)

if __name__ == "__main__":
  if(len(sys.argv) != 3):
    print "Input Arguments: <input PSD File> <output JSON File>"
  else:
    main()
  

