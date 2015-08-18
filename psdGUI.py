from easygui import *
import sys
import psd
import layerExport

while 1:
    msg ="What would you like to run?"
    title = "Creature PSD Pipeline Gui Exporter"
    choices = ["1) Export PSD layers into separate PNG images", "2) Generate Layout JSON from PSD"]
    choice = choicebox(msg, title, choices)

    if choice == choices[0]:
      psdFilename = fileopenbox(msg=None, title="Pick PSD File...",filetypes=["*.psd"])
      if psdFilename:
        exportFolder = diropenbox(title="Pick Folder to export to...")
        if(exportFolder):
          layerExport.writeLayerImages(psdFilename, exportFolder)

    elif choice == choices[1]:
      psdFilename = fileopenbox(msg=None, title="Pick PSD File...",filetypes=["*.psd"])
      if psdFilename:
        exportJSON = filesavebox(title="Pick JSON file to save to...",filetypes=["*.json"])
        if(exportJSON):
          psd.writeJSON(psdFilename, exportJSON)

    sys.exit(0)

