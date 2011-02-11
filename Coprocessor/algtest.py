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
import time
import traceback

def printError( message ):
	print "\033[1;31mError\033[0m -", message

# Check arguments
if len( sys.argv ) < 3:
	print "\033[1mUSAGE\033[0m: " + sys.argv[0] + " key script"
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
		printError( "Unable to load user script file." )
		if debug:
			raise
		exit()
		
	except:
		printError( "Unable to parse user script file." )
		if debug:
			raise
		exit()

if userScript is None:
	printError( "Unable to load user script." )
	exit()

# Battle-test the user script
totalMean = 0
totalVariance = 0
testsRun = 0
meanTime = 0

for (imageFile, ps) in tests:
	# Load the image file
	try:
		image = cv.LoadImage( imageFile )
	except:
		name, ext = os.path.splitext( imageFile )
		print "\033[1;31m" + name + "\033[0m - Unable to load file"
		continue
	
	# Run the user code
	identified = None
	startTime = None
	endTime = None
	try:
		startTime = time.time()
		identified = userScript.identifyTargets( image )
		endTime = time.time()
	except:
		name, ext = os.path.splitext( imageFile )
		exc_type, exc_value, exc_traceback = sys.exc_info()
		print "\033[1;31m" + name + "\033[0m - [\033[1m%s:%d\033[0m] '\033[3m%s\033[0m'" % (codeFile, exc_traceback.tb_lineno, exc_value)
		continue
	
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
	
	meanTime += (endTime - startTime)
	
	totalMean += mean
	totalVariance += sd ** 2
	
	name, ext = os.path.splitext( imageFile )
	print "\033[1;32m" + name + "\033[0m - Run Successful"
	if debug:
		print "    μ: %.2f" % mean
		print "    σ: %.2f" % sd
		print " time: %.2f" % (endTime - startTime)
	
	testsRun += 1

if testsRun == 0:
	printError( "No tests were run." )
	exit()

if debug:
	print "\n\033[1;34mOverall\033[0m"
	print "    μ: %.2f" % totalMean
	print "    σ: %.2f" % math.sqrt( totalVariance )
	print " time: %.2f" % meanTime

print ""

precision = 101 - 1.25 ** totalMean
accuracy  = 101 - 1.25 ** math.sqrt( totalVariance )
time      = 101 - 1.25 ** meanTime

print "\033[1mPrecision\033[0m: %d" % precision
print "\033[1m Accuracy\033[0m: %d" % accuracy
print "\033[1m     Time\033[0m: %d" % time

