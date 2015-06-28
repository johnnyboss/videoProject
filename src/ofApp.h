#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxDelaunay.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);		

    void reset();
    void reMap();
    void analyze();

    void initVideo();
	void pointSizeChanged(int & pointSize);
	void ringButtonPressed();

	bool bHide;

	ofxFloatSlider radius;

	ofxColorSlider color;
	ofxVec2Slider center;
	ofxIntSlider pointSize;
	ofxToggle redT, greenT, blueT, camera, original_image, rLineDraw, gLineDraw, bLineDraw, rXY, gXY, bXY, rTri, gTri, bTri;
	ofxButton twoCircles;
	ofxButton ringButton;
	ofxLabel screenSize;

	ofxPanel gui;
    
    ofFbo output;
    
    
    ofEasyCam cam;
    ofMesh Mesh;
    vector<ofMesh> rMesh, bMesh, gMesh;
    ofImage img;
    ofVideoPlayer video;
    ofVideoGrabber vidGrabber;
    class
	ofSoundPlayer ring;
    
    int rInterval, gInterval, bInterval;
    int point_size;
    
    vector<ofPoint> rTriangles;
    vector<ofPoint> gTriangles;
    vector<ofPoint> bTriangles;
    
    ofxDelaunay rtriangulation, gtriangulation, btriangulation;
    vector<int> rPrevious, gPrevious, bPrevious;
    vector<int> rCurrent, gCurrent, bCurrent;

};

