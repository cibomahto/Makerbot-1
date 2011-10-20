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
}

SimpleSkein::~SimpleSkein() {
}

void SimpleSkein::draw() {
	if (depthMapImage.getWidth() > 0) {
		depthMapImage.draw(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2);
	}
	
	glPushMatrix();
	for(int layer = 0; layer < sliceImages.size(); layer++) {
		glTranslated(0,0,200*layerHeight/numSamples);
		
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		sliceImages[layer].draw(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2);
		ofDisableBlendMode();
		
		glPushMatrix();
		glTranslated(-depthMapImage.getWidth()/2,-depthMapImage.getHeight()/2,.1);
		sliceContours[layer].draw();
		glPopMatrix();
	}
	glPopMatrix();
}

void SimpleSkein::makeGcode(std::string fname) {
	std::ofstream myfile;
	myfile.open ("/Users/mattmets/testout.gcode");
	
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
			for(int pointIndex = 0; pointIndex < points.size(); pointIndex++) {
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
	
	
	
	// Rough steps (from Kyle:)
	// 1. Get data from ofxKinect
	
	// 2. Load the data into an image for later display
	depthMapImage.setFromPixels(charPixels, width, height, OF_IMAGE_GRAYSCALE);
	
	
	// Clear the slice image buffer.
	drawSliceImages.clear();
	sliceImages.clear();
	sliceContours.clear();
	
	// for each layer
	float min = minScanDepth;
	float max = maxScanDepth;
	int steps = numSamples;

	
	// Build grayscale data for processing
	for (int step = 0; step < steps; step++) {
		float currentSliceHeight = (max - min)*((float)(steps - step)/steps) + min;
		
		// 3. build thresholded map
		ofImage layerImage;
		unsigned char charLayerPixels[height*width];
		for (uint i = 0; i < height*width; i++) {
			if (pixels[i] < currentSliceHeight) {
				charLayerPixels[i] = 30;
			}
			else {
				charLayerPixels[i] = 0;
			}
		}
		layerImage.setFromPixels(charLayerPixels, width, height, OF_IMAGE_GRAYSCALE);

		//   5. ofxCvContourFinder::findContours
		ofxCv::ContourFinder layerContour;
		layerContour.setThreshold(2);
		layerContour.findContours(layerImage);
		
		sliceImages.push_back(layerImage);
		sliceContours.push_back(layerContour);
	}

	
	
//	// Build RGBA data for display
//	for (int step = 0; step < steps; step++) {
//		float currentSliceHeight = (max - min)*((float)(steps - step)/steps) + min;
//		
//		// 3. build thresholded map
//		ofImage layerImage;
//		unsigned char charLayerPixels[height*width*4];
//		for (uint i = 0; i < height*width; i++) {
//			if (pixels[i] < currentSliceHeight) {
//				charLayerPixels[i*4+0] = ((float)step/steps)*255;
//				charLayerPixels[i*4+1] = (1-(float)step/steps)*255;
//				charLayerPixels[i*4+2] = 0;
//				charLayerPixels[i*4+3] = 255;
//			}
//			else {
//				charLayerPixels[i*4+0] = ((float)step/steps)*20;
//				charLayerPixels[i*4+1] = (1-(float)step/steps)*20;
//				charLayerPixels[i*4+2] = 0;
//				charLayerPixels[i*4+3] = 0;
//			}
//		}
//		layerImage.setFromPixels(charLayerPixels, width, height, OF_IMAGE_COLOR_ALPHA);
//		drawSliceImages.push_back(layerImage);
//	}

	
	//   6. ofxCvContourFinder::blobs
	// }
	// 7. convert to gcode.
};