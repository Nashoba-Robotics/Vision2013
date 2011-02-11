#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#	algorithmTest.py
#	Nashoba Robotics 2011
#	Computer Vision
#
#	Copyright 2011 RC Howe
#	All Rights Reserved
#

import cv
import hashlib
import imp
import math
import os.path
import sys
import traceback

# Check arguments
if len( sys.argv ) < 3:
	print "USAGE: " + sys.argv[0] + " key script"
	exit()

keyFile    = sys.argv[1]
scriptName = sys.argv[2]
debug = False
for arg in sys.argv:
	if arg == "-debug":
		debug = True

# Parse the manifest file
tests = []

with open( keyFile ) as f:
	lines = f.readlines()
	for line in lines:
		try:
			if line.strip() == '':
				continue
			
			imgFile, targets = tuple( line.split( ':' ) )
			points = map( lambda x: tuple( map( int, x.split( ',' ) ) ), targets.split( ';' ) )
			tests.append( (imgFile, points) )
		except:
			continue

# Load the user script
codeDir  = os.path.dirname(  scriptName )
codeFile = os.path.basename( scriptName )

userScript = None
with open( codeFile, 'rb' ) as fd:
	try:
		userScript = imp.load_source( hashlib.md5( codeFile ).hexdigest(), codeFile, fd )
	except ImportError, x:
		print "Unable to load file."
		raise
	except:
		print "Unable to parse file."
		raise

if userScript is None:
	print "Unable to load user script."
	exit()

# Battle-test the user script
totalMean = 0
totalVariance = 0

for (imageFile, ps) in tests:
	# Load the image file
	try:
		image = cv.LoadImage( imageFile )
	except:
		print "Unable to load image '%s'" % imageFile
		continue
	
	# Run the user code
	identified = None
	try:
		identified = userScript.identifyTargets( image )
	except:
		print "Error in user script"
		raise
	
	# Determine which targets are closest to those on the image
	matches = []
	totalDistance = 0
	points = list( ps )
	
	for (ix,iy) in identified:
		lowestDistance = None
		closestPoint   = None
		
		for (ax, ay) in points:
			distance = math.sqrt( (ax-ix) ** 2 + (ay-iy) ** 2 )
			if closestPoint is None or distance < lowestDistance:
				lowestDistance = distance
				closestPoint = (ax, ay)
		
		if lowestDistance is None:
			break
		
		x,y = closestPoint
		
		matches.append( ((ix,iy), closestPoint) )
		points.remove( closestPoint )
		
		totalDistance += lowestDistance
		
	# Calculate mean
	mean = 0
	if len( matches ) is not 0:
		mean = totalDistance / len( matches )
	
	# Calculate stddev
	sdtotal = 0
	for ((x,y),(ax,ay)) in matches:
		distance = math.sqrt( (ax-x) ** 2 + (ay-y) ** 2 )
		sdtotal += (distance - mean) ** 2
	
	sd = 0
	if len( matches ) is not 0:
		sd = math.sqrt( sdtotal / len( matches ) )
	
	totalMean += mean
	totalVariance += sd ** 2
	
	if debug:
		print "\n%s" % imageFile
		buf = '-' * len( imageFile )
		print buf
		print "    μ: %.2f" % mean
		print "    σ: %.2f" % sd

if debug:
	print "\nOverall"
	print "======="
	print "    μ: %.2f" % totalMean
	print "    σ: %.2f" % math.sqrt( totalVariance )

precision = 101 - 1.25 ** totalMean
accuracy  = 101 - 1.25 ** math.sqrt( totalVariance )

print "Precision: %d" % precision
print "Accuracy:  %d" % accuracy
