/*
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Copyright (C) 2000-2018, Intel Corporation, all rights reserved.
Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
Copyright (C) 2009-2016, NVIDIA Corporation, all rights reserved.
Copyright (C) 2010-2013, Advanced Micro Devices, Inc., all rights reserved.
Copyright (C) 2015-2016, OpenCV Foundation, all rights reserved.
Copyright (C) 2015-2016, Itseez Inc., all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

const int THRESHOLD_HARRIS = 125;

// Perform a single thinning iteration, which is repeated until the skeletization is finalized
void thinningIteration(Mat& im, int iter)
{
    Mat marker = Mat::zeros(im.size(), CV_8UC1);
    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);
            
            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
            (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
            (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
            (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);
            
            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }
    
    im &= ~marker;
}

// Function for thinning any given binary image 
void thinning(Mat& im)
{
    // Enforce the range of the input image to be in between 0 - 255
    im /= 255;
    
    Mat prev = Mat::zeros(im.size(), CV_8UC1);
    Mat diff;
    
    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (countNonZero(diff) > 0);
    
    im *= 255;
}

void compareImage(){

}

void detectMinutia(Mat& harris_corners, Mat& harris_normalised, Mat& input_thinned){
    harris_corners = Mat::zeros(input_thinned.size(), CV_32FC1);
    cornerHarris(input_thinned, harris_corners, 2, 3, 0.04, BORDER_DEFAULT);
    normalize(harris_corners, harris_normalised, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    
}

void detectKeyPoints(Mat& harris_normalised, Mat& rescaled, vector<KeyPoint>& keypoints,string name){
    convertScaleAbs(harris_normalised, rescaled);
    Mat harris_c(rescaled.rows, rescaled.cols, CV_8UC3);
    Mat in[] = { rescaled, rescaled, rescaled };
    int from_to[] = { 0,0, 1,1, 2,2 };
    mixChannels( in, 3, &harris_c, 1, from_to, 3 );
    for(int x=0; x<harris_normalised.cols; x++){
        for(int y=0; y<harris_normalised.rows; y++){
            if ( (int)harris_normalised.at<float>(y, x) > THRESHOLD_HARRIS ){
                // Draw or store the keypoint location here
                // In our case we will store the location of the keypoint

                //circle(harris_c, Point(x, y), 5, Scalar(0,255,0), 1);
                //circle(harris_c, Point(x, y), 1, Scalar(0,0,255), 1);

                keypoints.push_back( KeyPoint (x, y, 1) );
            }
        }
    }

    imshow(name, harris_c); waitKey(0);
}

void PerformMatching(){

}



int main( int argc, const char** argv )
{
    // Read in an input image - directly in grayscale CV_8UC1
    // This will be our test fingerprint
    Mat input = imread("/Users/nicholasrader/Documents/Nick/DB1_B/101_1.tif", IMREAD_GRAYSCALE);
    
    // Binarize the image, through local thresholding
    Mat input_binary;
    threshold(input, input_binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    
    // Now apply the thinning algorithm
    Mat input_thinned = input_binary.clone();
    thinning(input_thinned);
    
    // Now lets detect the strong minutiae using Haris corner detection
    Mat harris_corners, harris_normalised;
    detectMinutia(harris_corners,harris_normalised,input_thinned);
    
    // Select the strongest corners that you want
    vector<KeyPoint> keypoints;

    // detect keypoints of captured fingerprint
    Mat rescaled;
    detectKeyPoints(harris_normalised,rescaled,keypoints,"win1");
    
    
    // Calculate the ORB descriptor based on the keypoints captured
    Ptr<FeatureDetector> detector = ORB::create();
    
    Ptr<FeatureDetector> orb_descriptor = ORB::create();
    Mat descriptors;
    orb_descriptor->compute(input_thinned, keypoints, descriptors);
    
    // You can now store the descriptor in a matrix and calculate all for each image.
    // Since we just got the hamming distance brute force matching left, we will take another image and calculate the descriptors also.
    // Removed as much overburden comments as you can find them above
    Mat input2 = imread("/Users/nicholasrader/Documents/Nick/DB1_B/101_4.tif", IMREAD_GRAYSCALE);
    Mat input_binary2;
    threshold(input2, input_binary2, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    Mat input_thinned2 = input_binary2.clone();
    thinning(input_thinned2);

    Mat harris_corners2, harris_normalised2;
    detectMinutia(harris_corners2,harris_normalised2,input_thinned2) ;

    vector<KeyPoint> keypoints2;
    Mat rescaled2;
    detectKeyPoints(harris_normalised2,rescaled2,keypoints2,"win2");
    
    Mat descriptors2;
    orb_descriptor->compute(input_thinned2, keypoints2, descriptors2);
    
    //Now lets match both descriptors
    //Create the matcher interface
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    vector< DMatch > matches;
    matcher->match(descriptors, descriptors2, matches);
    
    // Loop over matches and multiply
    // Return the matching certainty score
    float score = 0.0;
    for(int i=0; i < matches.size(); i++){
        DMatch current_match = matches[i];
        score = score + current_match.distance;
    }
    cerr << endl << "Current matching score = " << score << endl;

    destroyAllWindows();
    
    return 0;
}
