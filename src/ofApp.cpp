#include "ofApp.h"

	
//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetVerticalSync(true);

	// we add this listener before setting up so the initial circle resolution is correct
	pointSize.addListener(this, &ofApp::pointSizeChanged);
	ringButton.addListener(this,&ofApp::ringButtonPressed);
    
	gui.setup(); // most of the time you don't need a name
	gui.add(redT.setup("redT", true));
    gui.add(greenT.setup("greenT", true));
    gui.add(blueT.setup("blueT", true));
    
    gui.add(rLineDraw.setup("r Line", false));
    gui.add(gLineDraw.setup("g Line", false));
    gui.add(bLineDraw.setup("b Line", false));
    
    gui.add(rXY.setup("rXY", false));
    gui.add(gXY.setup("gXY", false));
    gui.add(bXY.setup("bXY", false));

    gui.add(rTri.setup("rTri", false));
    gui.add(gTri.setup("gTri", false));
    gui.add(bTri.setup("bTri", false));

    gui.add(camera.setup("camara", false));
    gui.add(original_image.setup("original", false));
	gui.add(radius.setup( "radius", 140, 10, 300 ));
	gui.add(center.setup("center",ofVec2f(ofGetWidth()*.5,ofGetHeight()*.5),ofVec2f(0,0),ofVec2f(ofGetWidth(),ofGetHeight())));
	gui.add(color.setup("color",ofColor(100,100,140),ofColor(0,0),ofColor(255,255)));
	gui.add(pointSize.setup("point_size", 5, 3, 10));
	gui.add(twoCircles.setup("two circles"));
	gui.add(ringButton.setup("ring"));
	gui.add(screenSize.setup("screen size", ""));

	bHide = true;

	//ring.loadSound("ring.wav");
    
    video.loadMovie("IMG_0312.MOV");
    video.play();
    //video.setVolume(0);
    
    rInterval = 3;
    gInterval = 3;
    bInterval = 3;
    gui.loadFromFile("settings.xml");

    
    
   /* cameraTrack = new ofxTLCameraTrack();
    cameraTrack->setCamera(cam);
    timeline.addTrack("Camera", cameraTrack);
    
    cameraTrack->lockCameraToTrack = true;
    timeline.play();*/

    
  //  tweenelastic.setParameters(5,easingelastic,ofxTween::easeOut,0,ofGetWidth()-100,duration,delay);

    // 1. Create a new recorder object.  ofPtr will manage this
    // pointer for us, so no need to delete later.
   /* vidRecorder = ofPtr<ofQTKitGrabber>( new ofQTKitGrabber() );
    ofAddListener(vidRecorder->videoSavedEvent, this, &ofApp::videoSaved);
    vidRecorder->initRecording();*/

    ofFbo::Settings s;
    s.width			= video.getWidth();
    s.height			= video.getHeight();
    s.internalformat   = GL_RGBA32F_ARB;
    s.useDepth			= true;
    s.numSamples = 8;
    // and assigning this values to the fbo like this:
    output.allocate(s);
    output.begin();
    ofClear(255,255,255, 0);
    output.end();
    for(int i = 0; i < 640; i++)
    {
        for(int j = 0; j < 480; j++)
        {
            rPrevious.push_back(0);
            gPrevious.push_back(0);
            bPrevious.push_back(0);
            rCurrent.push_back(0);
            gCurrent.push_back(0);
            bCurrent.push_back(0);
        }
    }
    
    if(camera)
        initVideo();
}

void ofApp::initVideo()
{
    vidGrabber.setDeviceID(0);
    //vidGrabber.setDesiredFrameRate(60);
    vidGrabber.initGrabber(640,480);
    video.setPaused(true);
}
//--------------------------------------------------------------
void ofApp::exit()
{
	ringButton.removeListener(this,&ofApp::ringButtonPressed);
}

//--------------------------------------------------------------
void ofApp::pointSizeChanged(int & pointSize)
{
    point_size = pointSize;
	//ofSetCircleResolution(circleResolution);
}

//--------------------------------------------------------------
void ofApp::ringButtonPressed()
{
	ring.play();
}

//--------------------------------------------------------------
void ofApp::update()
{
    if(!vidGrabber.isInitialized() && camera)
        initVideo();
    
    if(camera)
        vidGrabber.update();
    else
        video.update();
    
    if(vidGrabber.isFrameNew() || video.isFrameNew())
        reMap();
}

void ofApp::reset()
{
    rMesh.clear();
    gMesh.clear();
    bMesh.clear();
    
    rTriangles.clear();
    gTriangles.clear();
    bTriangles.clear();
    
    
}

void ofApp::reMap()
{
    reset();
    if(camera)
        img.setFromPixels(vidGrabber.getPixels(), vidGrabber.getWidth(), vidGrabber.getHeight(), OF_IMAGE_COLOR);
    else
        img.setFromPixels(video.getPixels(), video.getWidth(), video.getHeight(), OF_IMAGE_COLOR);
 
    //img.resize(640, 480);
    output.begin();
    ofEnableDepthTest();
    glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
    glPointSize(point_size); // make the points bigger

    analyze();
    
    img.mirror(true, false);

    if(rTri)
    {
        rtriangulation.addPoints(rTriangles);
        rtriangulation.triangulate();
        rtriangulation.draw();
    }
    
    if(gTri)
    {
        gtriangulation.addPoints(gTriangles);
        gtriangulation.triangulate();
        gtriangulation.draw();

    }
    
    if(bTri)
    {
        btriangulation.addPoints(bTriangles);
        btriangulation.triangulate();
        btriangulation.draw();
    }
    output.end();
    }

void ofApp::analyze()
{
//    output.begin();
    
    // we're going to load a ton of points into an ofMesh
    
    ofClear(255,255,255, 0);
    // loop through the image in the x and y axes
    for(int y = 0; y < img.getHeight(); y += rInterval)
    {
        ofMesh rmesh_aux, gmesh_aux, bmesh_aux;
        if(rLineDraw)
            rmesh_aux.setMode(OF_PRIMITIVE_POINTS);
        else
            rmesh_aux.setMode(OF_PRIMITIVE_LINE_STRIP);

        if(gLineDraw)
            gmesh_aux.setMode(OF_PRIMITIVE_POINTS);
        else
            gmesh_aux.setMode(OF_PRIMITIVE_LINE_STRIP);

        if(bLineDraw)
            bmesh_aux.setMode(OF_PRIMITIVE_POINTS);
        else
            bmesh_aux.setMode(OF_PRIMITIVE_LINE_STRIP);

        for(int x = 0; x < img.getWidth(); x += rInterval)
        {
            int index = y*img.getWidth() + x;
            ofColor cur = img.getColor(x, y);
            cur.a = 255;

            if(redT)
            {
                rPrevious[index] = rCurrent[index];
                rmesh_aux.addColor(ofColor(cur.r, 0, 0));
                float rz = ofMap(cur.r*0.8, 0, 255, -100, 100);
                int z = rz + ((rz - rPrevious[index])/10);
                rCurrent[index] = z;
                ofVec3f posr(x, y, z);
                rmesh_aux.addVertex(posr);
            }
            
            if(greenT)
            {
                gPrevious[index] = gCurrent[index];
                gmesh_aux.addColor(ofColor(0, cur.g, 0));
                float rz = ofMap(cur.g*0.8, 0, 255, -100, 100);
                int z = rz + ((rz - gPrevious[index])/10);
                gCurrent[index] = z;
                ofVec3f posg(x, y, z);
                gmesh_aux.addVertex(posg);
            }
            
            if(blueT)
            {
                rPrevious[index] = rCurrent[index];
                bmesh_aux.addColor(ofColor(0, 0, cur.b));
                float rz = ofRandom(x,y);
//                float rz = ofMap(cur.b*0.8, 0, 255, -100, 100) + ofRandom(10);
                int z = rz + ((rz - rPrevious[index])/10);
                rCurrent[index] = z;
                ofVec3f posb(x, y, z);
                bmesh_aux.addVertex(posb);
            }
            
            
        }
//        rmesh_aux.draw();
//        bmesh_aux.draw();
//        gmesh_aux.draw();
//
        rMesh.push_back(rmesh_aux);
        gMesh.push_back(gmesh_aux);
        bMesh.push_back(bmesh_aux);

    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0, 0, 0);
    // auto draw?
    // should the gui control hiding?
    
    int fadeAmnt = 15;
    //1 - Fade Fbo
    
    //this is where we fade the fbo
    //by drawing a rectangle the size of the fbo with a small alpha value, we can slowly fade the current contents of the fbo.
    ofEnableAlphaBlending();
    
    

    ofPushMatrix();
    ofDisableDepthTest();
    if( bHide ){
        gui.draw();
        
        ofSetColor(255,0,255);
        ofCircle(ofGetMouseX(), ofGetMouseY(),5);
    }
    ofPopMatrix();
    ofEnableDepthTest();
    

    ofQuaternion startQuat;
    ofQuaternion targetQuat;
    ofVec3f startPos;
    ofVec3f targetPos;
    
    // we define the camer's start and end orientation here:
    startQuat.makeRotate(0, 0, 1, 0);			// zero rotation.
    targetQuat.makeRotate(90, 0, 1, 0);			// rotation 90 degrees around y-axis.
    
    // we define the camer's start and end-position here:
    startPos.set(0,0,0);
    targetPos.set(0,0,600);
    
    // ofCamera myCam;
    float tweenvalue = (ofGetElapsedTimeMillis() % 2000) /2000.f; // this will slowly change from 0.0f to 1.0f, resetting every 2 seconds

    ofQuaternion tweenedCameraQuaternion;	// this will be the camera's new rotation.
    
    // calculate the interpolated orientation
    tweenedCameraQuaternion.slerp(tweenvalue, startQuat, targetQuat);
    
    ofVec3f lerpPos;					//this will hold our tweened position.
    
    // calculate the interpolated values.
    lerpPos.x = ofLerp(tweenvalue, startPos.x, targetPos.x);
    lerpPos.y = ofLerp(tweenvalue, startPos.y, targetPos.y);
    lerpPos.z = ofLerp(tweenvalue, startPos.z, targetPos.z);
    
    // alternative way to calculate interpolated values:
    // lerpPos = startPos + ((targetPos-startPos) * tweenvalue);
    
    // now update the camera with the calculated orientation and position.
//    cam.setOrientation(tweenedCameraQuaternion);
//    cam.setGlobalPosition(lerpPos);

    cam.begin();
//    cam.tilt(ofMap(ofGetMouseX(), 0, 100, -50, 50));


//    ofScale(2, -2, 2); // flip the y axis and zoom in a bit
//    ofTranslate(-img.getWidth() / 2, -img.getHeight() / 2);
    if(original_image)
    {
        ofPushMatrix();
        ofSetColor(255,255,255);
        ofTranslate(0,0, -100);
        img.draw(0,0);
        ofPopMatrix();
    }
   
    
   //    output.draw(0, 0);
    
    for(int i = 0; i < rMesh.size(); i++)
    {
        if(redT)
            rMesh[i].draw();
            
        if(greenT)
            gMesh[i].draw();
        
        if(blueT)
            bMesh[i].draw();
    }
    
    cam.end();
   
   // timeline.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if( key == 'h' )
    {
		bHide = !bHide;
	}
	if(key == 's')
    {
		gui.saveToFile("settings.xml");
	}
	if(key == 'l')
    {
		gui.loadFromFile("settings.xml");
	}
	if(key == ' ')
    {
		color = ofColor(255);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
    screenSize = ofToString(w) + "x" + ofToString(h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
	
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
	
}
