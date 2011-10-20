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
using namespace ofxCv;
using namespace cv;

/**
 * Take a depth map and build an object shell from it.
 */
class SimpleSkein {
private:
	ofImage depthMapImage; // 2d depth map
	
	vector<ofImage> drawSliceImages;  // Slice data, in RGBA.

	vector<ofImage> sliceImages;  // Slice data, in grayscale.

	vector<ofxCv::ContourFinder> sliceContours;
	
	float layerHeight;
	
	float maxDepth;
	float minDepth;

public:
    SimpleSkein();
	~SimpleSkein();

	float minScanDepth;
	float maxScanDepth;
	
	int numSamples;
	
	float getMaxDepth() { return maxDepth; }
	float getMinDepth() { return minDepth; }
	
	/**
	 * Draw a representation of the skeined object to the screen.
	 */
	void draw();
	
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