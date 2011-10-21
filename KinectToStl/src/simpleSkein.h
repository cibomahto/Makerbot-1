/*
 *  simpleSkein.h
 *  KinectToStl
 *
 *  Created by Matt Mets on 10/19/11.
 *  Copyright 2011 Matt Mets. All rights reserved.
 *
 */

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
using namespace ofxCv;
using namespace cv;

/**
 * Take a depth map and build an object shell from it.
 */
class SimpleSkein {
private:
	ofImage depthMapImage; // 2d depth map

	vector<ofImage> sliceImages;  // Slice data, in grayscale.
	vector<ofxCvGrayscaleImage> infillImages;  // Infill data, in grayscale.
	
	vector<ofxCv::ContourFinder> sliceContours;		// Contours for external shells
	vector<ofxCv::ContourFinder> infillContours;	// Contours for infill
	
	float layerHeight;
	
	float maxDepth;
	float minDepth;

public:
    SimpleSkein();
	~SimpleSkein();

	float minScanDepth;	// Minimum depth to consider
	float maxScanDepth;	// Maximum depth to consider
	
	int numSamples;		// Number of layer slices to cut the region of interest in our depth map to
	
	int infillGridSize;	// Size of the infill grid to paint
	
	float getMaxDepth() { return maxDepth; }
	float getMinDepth() { return minDepth; }
	
	/**
	 * Draw a representation of the skeined object to the screen.
	 */
	void draw(bool drawDepth,
			  bool drawSliceThresholds, bool drawSliceContours,
			  bool drawInfillThresholds, bool drawInfillContours);

	
	/**
	 * Save a gcode file out, good for printing the object.
	 */
	void makeGcode(std::string fname);

	/**
	 * Skein a depth map
	 * @param height Height of image, in pixels
	 * @param width Width of image, in pixels
	 * @param pixels depth array, from top left (?), in cm
	 * @param layerHeight height of each slice, in cm, from 0 up.
	 */
    void skeinDepthMap(int height, int width, float* pixels, float layerHeight);
};