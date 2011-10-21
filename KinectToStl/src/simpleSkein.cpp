/*
 *  simpleSkein.cpp
 *  KinectToStl
 *
 *  Created by Matt Mets on 10/19/11.
 *  Copyright 2011 Matt Mets. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>


#include "simpleSkein.h"

SimpleSkein::SimpleSkein() {
	// some defaults.
	minScanDepth = 40;
	maxScanDepth = 65;
	numSamples = 10;
	infillGridSize = 20;
}

SimpleSkein::~SimpleSkein() {
}

void SimpleSkein::draw(bool drawDepth,
					   bool drawSliceThresholds, bool drawSliceContours,
					   bool drawInfillThresholds, bool drawInfillContours) {
	
	
	if (drawDepth) {
		depthMapImage.draw(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2);
	}
	
	glPushMatrix();
	for(int layer = 0; layer < sliceImages.size(); layer++) {
		glTranslated(0,0,200*layerHeight/numSamples);

		if (drawSliceThresholds) {
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			sliceImages[layer].draw(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2);
			ofDisableBlendMode();
		}
		
		if (drawSliceContours) {
			ofSetColor(0,0,200);
			glPushMatrix();
			glTranslated(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2,.1);
			sliceContours[layer].draw();
			glPopMatrix();
			ofSetColor(255,255,255);
		}
		
		if (drawInfillThresholds) {
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			infillImages[layer].draw(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2);
			ofDisableBlendMode();
		}
		
		if (drawInfillContours) {
			ofSetColor(0,0,200);
			glPushMatrix();
			glTranslated(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2,.1);
			infillContours[layer].draw();
			glPopMatrix();
			ofSetColor(255,255,255);
		}
	}
	glPopMatrix();
}

void SimpleSkein::makeGcode(std::string fname) {
	std::ofstream myfile;
	myfile.open ("/Users/mattmets/testout.gcode");
	
	// add start codes
	
	myfile << "(**** beginning of start.gcode ****)" << std::endl;
	myfile << "(This file is for a MakerBot Thing-O-Matic)" << std::endl;
	myfile << "(**** begin initialization commands ****)" << std::endl;
	myfile << "G21 (set units to mm)" << std::endl;
	myfile << "G90 (set positioning to absolute)" << std::endl;
	myfile << "M108 R5.0 (set extruder speed)" << std::endl;
	myfile << "M103 (Make sure extruder is off)" << std::endl;
	myfile << "M104 S225 T0 (set extruder temperature)" << std::endl;
	myfile << "M109 S100 T0 (set heated-build-platform temperature)" << std::endl;
	myfile << "(**** end initialization commands ****)" << std::endl;
	myfile << "(**** begin homing ****)" << std::endl;
	myfile << "G162 Z F1000 (home Z axis maximum)" << std::endl;
	myfile << "G92 Z10 (set Z to 0)" << std::endl;
	myfile << "G1 Z0.0 (move z down 10)" << std::endl;
	myfile << "G162 Z F150 (home Z axis maximum)" << std::endl;
	myfile << "G161 X Y F2500 (home XY axes minimum)" << std::endl;
	myfile << "M132 X Y Z A B (Recall stored home offsets for XYZAB axis)" << std::endl;
	myfile << "(**** end homing ****)" << std::endl;
	myfile << "(**** begin pre-wipe commands ****)" << std::endl;
	myfile << "G1 X57.0 Y-57.0 Z10.0 F3300.0 (move to waiting position)" << std::endl;
	myfile << "M6 T0 (wait for toolhead parts, nozzle, HBP, etc., to reach temperature)" << std::endl;
	
	
	
	char buff[200];
	
	// Skein variables
	float feedrate = 1800;		// in mm/s- this is fixed.
	float flowrate = 1.573;		// in RPM
	float layerHeight = .3;		// in mm
	float zOffset = .78;		// in mm - added to all z coordinates.

	// Scaling pixels to mm
	float scale = .1;
	float xShift = -depthMapImage.getWidth()/2;
	float yShift = -depthMapImage.getHeight()/2;

	
	
	// For each layer
	for(int layer = 0; layer < sliceContours.size(); layer++) {
		myfile << "(On layer: " << layer << ")" << std::endl;
		
		// For each contour on the slice
		for(int contour = 0; contour < sliceContours[layer].size(); contour++) {
			myfile << "(On contour: " << contour << ")" << std::endl;
			
			vector<cv::Point> points = sliceContours[layer].getContour(contour);
			
			// Move to the first point in the contour
			sprintf(buff, "G1 X%03f Y%03f Z%03f F%03f",
				    (points[0].x+xShift)*scale,
				    (points[0].y+yShift)*scale,
				    (double)layer*layerHeight + zOffset,
				    feedrate);
			myfile << buff << std::endl;
				  
			// Turn the extruder on
			myfile << "M101" << std::endl;
			
			// Move to each point in the contour
			for(int pointIndex = 1; pointIndex < points.size(); pointIndex++) {
				sprintf(buff, "G1 X%03f Y%03f Z%03f F%03f",
						(points[pointIndex].x+xShift)*scale,
						(points[pointIndex].y+yShift)*scale,
						(double)layer*layerHeight + zOffset,
						feedrate);
				myfile << buff << std::endl;
			}
			
			// Turn the extruder off
			myfile << "M103" << std::endl;
		}
		
		// For each contour on the infill
		for(int contour = 0; contour < infillContours[layer].size(); contour++) {
			myfile << "(On contour: " << contour << ")" << std::endl;
			
			vector<cv::Point> points = infillContours[layer].getContour(contour);
			
			// Move to the first point in the contour
			sprintf(buff, "G1 X%03f Y%03f Z%03f F%03f",
				    (points[0].x+xShift)*scale,
				    (points[0].y+yShift)*scale,
				    (double)layer*layerHeight + zOffset,
				    feedrate);
			myfile << buff << std::endl;
			
			// Turn the extruder on
			myfile << "M101" << std::endl;
			
			// Move to each point in the contour
			for(int pointIndex = 1; pointIndex < points.size(); pointIndex++) {
				sprintf(buff, "G1 X%03f Y%03f Z%03f F%03f",
						(points[pointIndex].x+xShift)*scale,
						(points[pointIndex].y+yShift)*scale,
						(double)layer*layerHeight + zOffset,
						feedrate);
				myfile << buff << std::endl;
			}
			
			// Turn the extruder off
			myfile << "M103" << std::endl;
		}
	}
	
	// Add end codes
	myfile << "(**** Beginning of end.gcode ****)" << std::endl;
	myfile << "(This file is for a MakerBot Thing-O-Matic)" << std::endl;
	myfile << "(*** begin settings ****)" << std::endl;
	myfile << "(**** end settings ****)" << std::endl;
	myfile << "(**** begin move to cooling position ****)" << std::endl;
	myfile << "G1 X0.0 F3300.0       (move to cooling position)" << std::endl;
	myfile << "G1 X0.0 Y55.0 F3300.0 (move to cooling position)" << std::endl;
	myfile << "G162 Z F1000          (home Z axis maximum and begin cooling)" << std::endl;
	myfile << "(**** end move to cooling position ****)" << std::endl;
	myfile << "(**** begin filament reversal ****)" << std::endl;
	myfile << "M108 R1.98" << std::endl;
	myfile << "M102 (Extruder on, reverse)" << std::endl;
	myfile << "G04 P2000 (Wait t/1000 seconds)" << std::endl;
	myfile << "M108 R1.98" << std::endl;
	myfile << "M103 (Extruder off)" << std::endl;
	myfile << "(**** end filament reversal ****)" << std::endl;
	myfile << "M18 (Turn off steppers)" << std::endl;
	myfile << "(**** begin eject ****)" << std::endl;
	myfile << "(**** end eject ****)" << std::endl;
	myfile << "(**** begin cool for safety ****)" << std::endl;
	myfile << "M104 S0 T0 (set extruder temperature)" << std::endl;
	myfile << "M109 S0 T0 (set heated-build-platform temperature)" << std::endl;
	myfile << "(**** end cool for safety ****)" << std::endl;
	myfile << "(**** end of end.gcode ****)" << std::endl;
	myfile << "(</alteration>)" << std::endl;
	myfile << ";M113 S0.0" << std::endl;
	
	
	myfile.close();
}

void SimpleSkein::skeinDepthMap(int height, int width, float* pixels, float layerHeight_) {
	// We want to go through the depth map, making slices at various layer heights and then converting
	// them into movements. Facile!
	
	layerHeight = layerHeight_;
	
	maxDepth = minDepth = pixels[0];
	
	// First, downsample the data into 8- bits.
	unsigned char charPixels[height*width];
	for (uint i = 0; i < height*width; i++) {
		if (pixels[i] > maxDepth) {
			maxDepth = pixels[i];
		}
		if (pixels[i] < minDepth) {
			minDepth = pixels[i];
		}
		
		charPixels[i] = pixels[i]/2;
	}
//	
//	// 2. Load the data into an image for later display
	depthMapImage.setFromPixels(charPixels, width, height, OF_IMAGE_GRAYSCALE);

	// Clear the slice image buffer.
	sliceImages.clear();
	sliceContours.clear();
	infillImages.clear();
	infillContours.clear();
	
	// for each layer
	float min = minScanDepth;
	float max = maxScanDepth;
	int steps = numSamples;

	
	// Detect contours along the edges
	for (int step = 0; step < steps; step++) {
		float currentSliceHeight = (max - min)*((float)(steps - step)/steps) + min;
		
		// 3. build thresholded map
		ofImage layerImage;
		unsigned char charLayerPixels[height*width];
		for (uint i = 0; i < height*width; i++) {
			if (pixels[i] < currentSliceHeight) {
				charLayerPixels[i] = 15;
			}
			else {
				charLayerPixels[i] = 0;
			}
		}
		layerImage.setFromPixels(charLayerPixels, width, height, OF_IMAGE_GRAYSCALE);
		
		// Find the contours for the image
		ofxCv::ContourFinder layerContour;
		layerContour.setThreshold(1);
		layerContour.findContours(layerImage);
		
		sliceImages.push_back(layerImage);
		sliceContours.push_back(layerContour);
	}
	
	// Now, add a checkerboard mask and then erode the image to generate infill.
	
	// We want to make two sets of rectangles in a checkerboard pattern.
	for (int yScan = 0; yScan < height; yScan++) {
		for (int xScan = 0; xScan < width; xScan++) {
			// If we are on an even set of rows, we want to blank out the first rectangle.
			if (yScan %(infillGridSize*2) < infillGridSize) {
				if (xScan %(infillGridSize*2) < infillGridSize) {
					// First, just blank out the whole line.
					pixels[yScan*width + xScan] = 9999;
				}
			}
			else {
				if (xScan %(infillGridSize*2) >= infillGridSize) {
					// First, just blank out the whole line.
					pixels[yScan*width + xScan] = 9999;
				}
			}
		}
	}
	
	// And then chop off the edges of them as well, so that the checkers aren't connected.
	for (int yScan = 0; yScan < height; yScan+=infillGridSize) {
		for (int xScan = 0; xScan < width; xScan+=1) {
			pixels[yScan*width + xScan] = 9999;
		}
	}
	
	for (int xScan = 0; xScan < width; xScan+=infillGridSize) {
		for (int yScan = 0; yScan < height; yScan+=1) {
			pixels[yScan*width + xScan] = 9999;
		}
	}
	
	
	// Detect contours for the infill.
	for (int step = 0; step < steps; step++) {
		float currentSliceHeight = (max - min)*((float)(steps - step)/steps) + min;
		
		// 3. build thresholded map
		ofxCvGrayscaleImage layerImage;
		
		unsigned char charLayerPixels[height*width];
		for (uint i = 0; i < height*width; i++) {
			if (pixels[i] < currentSliceHeight) {
				charLayerPixels[i] = 15;
			}
			else {
				charLayerPixels[i] = 0;
			}
		}
		
		layerImage.allocate(width, height);
		layerImage.setFromPixels(charLayerPixels, width, height);
		
		// Erode the image to generate an offset for infill
		layerImage.erode();
		
		// Find the contours for the image
		ofxCv::ContourFinder layerContour;
		layerContour.setThreshold(1);
		layerContour.findContours(layerImage);
		
		infillImages.push_back(layerImage);
		infillContours.push_back(layerContour);
	}
};