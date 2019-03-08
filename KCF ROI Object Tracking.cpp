//
//  main.cpp
//  KCF_tracker
//
//  Created by Nicholas Rader on 2018/10/09.
//  Copyright © 2018 Nicholas Rader. All rights reserved.
//
#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <iostream>
#include <cstring>
using namespace std;
using namespace cv;

static void KCF(){
    // declares all required variables
    Rect2d roi;
    Mat frame;
    bool editMode = false;
    bool getframes = true;
    
    // create a tracker object
    Ptr<TrackerKCF> tracker = TrackerKCF::create();
    
    // capture from the webcam
    VideoCapture cap(0);
    
    if (!cap.isOpened()) { //check if video device has been initialised
        cout << "cannot open camera";
    }
    
    // capture first 10 frames to get lighting sorted etc
    for(int i = 0;i < 10 ; i++){
        // get bounding box from first frame
        cap.read(frame);
    }
    roi = selectROI("tracker",frame);
    //quit if ROI was not selected
   
    // initialize the tracker
    tracker->init(frame,roi);
    
    // perform the tracking process
    printf("Start the tracking process, press ESC to quit.\n");
    
    while(getframes){
        
        cap >> frame;
        // stop the program if no more images
        if(frame.rows == 0 || frame.cols == 0){
            break;
        }
        
        // update the tracking result
        tracker->update(frame,roi);
        
        // draw the tracked object
        rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
        // show image with the tracked object
        imshow("tracker",frame);
        
        switch(waitKey(10)){
            case 27:
                //'esc' key has been pressed, exit program.
                getframes = false;
                break;
            case 116:
                //'t' has been pressed. this will toggle editmode
                editMode = !editMode;
                if(editMode == false) {
                    cout << "Edit Mode off." << endl;
                }else {
                    cout << "Edit mode on" << endl;
                    roi = selectROI("tracker",frame);
                    // get frame from the video
                    tracker->update(frame,roi);
                    rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
                }
                break;
        }
    }
    
   
}

int main( int argc, char** argv ){

    return 0;
}
