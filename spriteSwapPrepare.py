# Script to allow you to resize a group fo selected images to the correct aspect ratio suitable for Sprite Swapping onto a target Mesh in Creature
# Find the aspect ratio in Creature by selecting the target mesh in Animation Mode and then clicking the Tools -> Determine Sprite Dimension menu.

from PIL import Image, ImageChops
from os import listdir
import os
from os.path import isfile, join
from easygui import *

def getFilterFiles(folderIn, filterName):
	onlyfiles = [ f for f in listdir(folderIn) if isfile(join(folderIn,f)) ]
	filterFiles = []

	for curFile in onlyfiles:
		extension = os.path.splitext(curFile)[1][1:]
		if extension == filterName:
			filterFiles.append(folderIn + curFile)

	return filterFiles

def trim(im):
	imageBox = im.getbbox()
	return im.crop(imageBox)

def resizeToAspectRatio(im, targetRatio, fudgeFactor, offsetX, offsetY):
	im = trim(im)

	if fudgeFactor < 1.0:
		fudgeFactor = 1.0

	localWidth, localHeight = im.size
	localRatio = float(localWidth) / float(localHeight)

	targetWidth = localWidth
	targetHeight = localHeight

	if targetRatio > localRatio:
		scaleFactor = targetRatio / localRatio
		targetWidth = localWidth * scaleFactor
	else:
		localInvRatio = 1.0 / localRatio
		targetInvRatio = 1.0 / targetRatio
		scaleFactor = targetInvRatio / localInvRatio

		targetHeight = localHeight * scaleFactor

	targetWidth *= fudgeFactor
	targetHeight *= fudgeFactor

	targetWidth += 2
	targetHeight += 2

	pasteCenterX = (targetWidth - 1) * 0.5
	pasteCenterY = (targetHeight - 1) * 0.5
	pasteOriginX = int(pasteCenterX - localWidth * 0.5)
	pasteOriginY = int(pasteCenterY - localHeight * 0.5)

	newIm = Image.new('RGBA', (int(targetWidth), int(targetHeight)), (0,0,0,0))

	newDest = (pasteOriginX + offsetX, pasteOriginY + offsetY)
	newIm.paste(im, box=newDest)

	return newIm

def resizeGroupImages(srcFolderPath, dstFolderPath, targetRatio, fudgeFactor, offsetX, offsetY):
	if not os.path.exists(dstFolderPath):
		os.makedirs(dstFolderPath)

	allFiles = getFilterFiles(srcFolderPath, "png")
	for curFile in allFiles:
		newFilename = dstFolderPath + "/" + os.path.basename(curFile)

		srcImage = Image.open(curFile)
		srcImage.load()

		dstImge = resizeToAspectRatio(srcImage, targetRatio, fudgeFactor, offsetX, offsetY)
		dstImge.save(newFilename)

	return len(allFiles)

srcFolder = diropenbox(title="Pick Source Folder with PNG Images...")
if srcFolder:
	dstFolder = diropenbox(title="Pick Destination Folder to save PNG Images...")
	if dstFolder:
		aspectRatio = enterbox(msg="Enter the target aspect ratio (Get this value in Creature from: Tools-> Determine Sprite Dimensions)", default="1.0")
		if aspectRatio:
			bufferFactor = enterbox(msg="Enter a larger Buffer Factor to pad the images and scale them smaller ( put in a value > 1.0):", default="1.05")
			if bufferFactor:
				offsetTitle = "Offset Values"
				fieldNames = ["Offset X", "Offset Y"]
				fieldValues = ["0", "0"]
				fieldValues = multenterbox("Enter Positional Offset Values if you want to shift the images:", offsetTitle, fieldNames, fieldValues)

				if fieldValues:
					offsetX = int(fieldValues[0])
					offsetY = int(fieldValues[1])

					numFiles = resizeGroupImages(srcFolder + "\\", dstFolder + "\\", float(aspectRatio), float(bufferFactor), offsetX, offsetY)
					print "Wrote %s files into directory %s" %(numFiles, dstFolder)
