/*
 *  simpleSkein.h
 *  KinectToStl
 *
 *  Created by Matt Mets on 10/19/11.
 *  Copyright 2011 Matt Mets. All rights reserved.
 *
 */


#include <iostream>
#include <fstream>

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

	vector<ofxCvGrayscaleImage> sliceImages;  // Slice data, in grayscale. One per layer.
	vector<ofxCvGrayscaleImage> infillImages;  // Infill data, in grayscale. Two per layer (A mask and B mask).
	vector<ofxCvGrayscaleImage> topfillImages;  // Topfill data, in grayscale. One per layer.
	
	vector<ofxCv::ContourFinder> sliceContours;		// Contours for external shells, one per shell per layer.
	vector<ofxCv::ContourFinder> infillContours;	// Contours for infill, two per (A mask and B mask).
	vector<ofxCv::ContourFinder> topfillContours;	// Contours for topfill. One per layer.
	
	float maxDepth;
	float minDepth;

	/**
	* Calculate the slice height for the current sample
	* @param[in] sample slice number
	* @return Layer height, in mm
	*/
	float getHeightForSample(int sample);
	
	/**
	 * Write a contour (single extruded path) out to a file
	 * @param[out] file File stream to write to
	 * @param[in] points List of points to draw
	 * @param[in] layer Layer number
	 */
	void writeContour(std::ofstream& file, vector<cv::Point> points, int layer);

public:
    SimpleSkein();
	~SimpleSkein();

	float minScanDepth;	// Minimum depth to consider
	float maxScanDepth;	// Maximum depth to consider
	
	int numSamples;		// Number of layer slices to cut the region of interest in our depth map to
	
	int numShells;		// Number of outside layers to make
	int infillGridSize;	// Size of the infill grid to paint
		
	// Skein variables
	float feedrate;		// in mm/s- this is fixed.
	float flowrate;		// in RPM
	float layerHeight;	// in mm
	float zOffset;		// in mm - added to all z coordinates.
	
	// Reversal variables
	float reversalTime;	// Amount of time to reverse after extrusion ends (ms)
	float reversalRPM;	// Speed to reverse at (RPM)
	float pushbackTime;	// Amount of time to push back before extrusion starts (ms)
	float pushbackRPM;	// Speed to push back at (RPM)
	
	
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
	 */
    void skeinDepthMap(int height, int width, float* pixels);
};