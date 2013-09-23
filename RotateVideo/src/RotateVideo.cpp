/*
 * RotateVideo.cpp
 *
 *  Created on: 2013-06-14
 *      Author: nehiljain
 */

#include <iostream>
#ifdef linux
	#include <stdio.h>
#endif

#define USE_PPHT
// #undef USE_PPHT
#define MAX_NUM_LINES	200

#include <opencv2/opencv.hpp>
 #include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

/*
*GLOBAL VARIABLES
*
*/
// int lowerH=0;
// int lowerS=0;
// int lowerV=0;

// int upperH=180;
// int upperS=256;
// int upperV=256;



// void setwindowSettings(){
 	
//  	cvNamedWindow("Video");
//  	cvNamedWindow("Ball");
//     cvCreateTrackbar("LowerH", "Ball", &lowerH, 180, NULL);
//     cvCreateTrackbar("UpperH", "Ball", &upperH, 180, NULL);
//     cvCreateTrackbar("LowerS", "Ball", &lowerS, 256, NULL);
//     cvCreateTrackbar("UpperS", "Ball", &upperS, 256, NULL);
//     cvCreateTrackbar("LowerV", "Ball", &lowerV, 256, NULL);
//     cvCreateTrackbar("UpperV", "Ball", &upperV, 256, NULL); 
// }


/*
void generateThreasholdValues(cv::Mat &inputtImg){
	
	printf("Generate the threshold Values");
	cv::Mat hsvImg, imgBinary;
	cvNamedWindow("HSV", CV_WINDOW_AUTOSIZE );
	cvNamedWindow("Binary", CV_WINDOW_AUTOSIZE );
	setwindowSettings();
	cvtColor(inputtImg,hsvImg,CV_RGB2HSV);
	imshow("HSV", hsvImg);
	
	//converting from mat to IplImage and vice versa
    IplImage* pI = &I.operator IplImage();
    CvMat* mI = &I.operator CvMat();

	// change this functoion 
	cvInRange(hsvImg, cvScalar(lowerH,lowerS,lowerV), cvScalar(upperH,upperS,upperV), imgBinary); 
	imshow("Binary", hsvImg);
	
}
*/

//Canny edge detector 

// Global variables
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "EdgeMap";
Mat src_gray;
RNG rng(12345);
/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold( int, void* )
{
  Mat detected_edges;			
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

  imshow( window_name, detected_edges );
 }

// This is a method to find contours and overlay on the image
// This method is not useful in zerbracrossin detection. Hence commented
// void DrawContours(cv::Mat &imgCanny, cv::Mat &outputImg) {
 	
//  	cv::Mat imgContour;
//  	//sequences for Contours might be useless later on
// 	vector< vector<Point> > contours;
// 	vector<vector<Point> > contours_poly( contours.size() );
//   	vector<Rect> boundRect( contours.size() );
//   	imgCanny.copyTo(imgContour);
//   	//contours
// 	findContours(imgContour, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	
// 	int minArea = 500, maxArea = 100000; 

// 	// keep only contours of a certain size
// 	for (vector<vector<Point> >::iterator it=contours.end(); it!=contours.begin(); it--) {
// 	    if ((*it).size()<minArea || (*it).size()>maxArea) {
// 	        contours.erase(it);
// 	    }
// 	}

// 	// cv::drawContours(imgContour, contours, -1, cvScalar(255,0,0), CV_FILLED);
// 	cvNamedWindow("Contours", CV_WINDOW_AUTOSIZE );
// 	imshow("Contours", imgContour);

//  }

/** This function contains the actions performed for each image*/
void processImage(cv::Mat &imgGRAY, cv::Mat &outputImg)
{
	cv::Mat imgCanny, imgThresh;

	// Canny
	cv::Canny(imgGRAY, imgCanny, 70, 200, 3);
	cvNamedWindow("EdgeMap", CV_WINDOW_AUTOSIZE );
	imshow("EdgeMap", imgCanny);
	
	// Hough
	vector<vector<cv::Point> > lineSegments;
	vector<cv::Point> aux;
#ifndef USE_PPHT
	vector<Vec2f> lines;
	cv::HoughLines( imgCanny, lines, 1, CV_PI/180, 200);

	for(size_t i=0; i< lines.size(); i++)
	{
		float rho = lines[i][0];
		float theta = lines[i][1];

		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;

		Point pt1, pt2;
		pt1.x = cvRound(x0 + 1000*(-b));
		pt1.y = cvRound(y0 + 1000*(a));
		pt2.x = cvRound(x0 - 1000*(-b));
		pt2.y = cvRound(y0 - 1000*(a));

		aux.clear();
		aux.push_back(pt1);
		aux.push_back(pt2);
		lineSegments.push_back(aux);

		line(outputImg, pt1, pt2, CV_RGB(255, 0, 0), 1, 8);
	
	}
#else
	vector<Vec4i> lines;	
	int houghThreshold = 70;
	if(imgGRAY.cols*imgGRAY.rows < 400*400)
		houghThreshold = 100;		
	
	cv::HoughLinesP(imgCanny, lines, 1, CV_PI/180, houghThreshold, 10,10);

	while(lines.size() > MAX_NUM_LINES)
	{
		lines.clear();
		houghThreshold += 10;
		cv::HoughLinesP(imgCanny, lines, 1, CV_PI/180, houghThreshold, 10, 10);
	}
	for(size_t i=0; i<lines.size(); i++)
	{		
		Point pt1, pt2;
		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];

		//calculate the slope of the line
		double theta = atan( (double)(pt2.y - pt1.y)/(pt2.x - pt1.x) ); /*slope of line*/
        double degree = theta*180/CV_PI;
      
		printf("angle:%f", degree);        
		
		// line(outputImg, pt1, pt2, CV_RGB(255,0,0), 2);


//replace the degrees of the lines properly with statistical measurements
		if(fabs(degree)<100 && fabs(degree) > 70){
			line(outputImg, pt1, pt2, CV_RGB(255,0,0), 2);
		}
		
		/*circle(outputImg, pt1, 2, CV_RGB(255,255,255), CV_FILLED);
		circle(outputImg, pt1, 3, CV_RGB(0,0,0),1);
		circle(outputImg, pt2, 2, CV_RGB(255,255,255), CV_FILLED);
		circle(outputImg, pt2, 3, CV_RGB(0,0,0),1);*/

		// Store into vector of pairs of Points for msac
		aux.clear();
		aux.push_back(pt1);
		aux.push_back(pt2);
		lineSegments.push_back(aux);
	}
	
#endif

}

//This method should be replaced by actual Horizon calculation beased on the acelerator values

void drawHorizon(cv::Mat &inputImg){
	//draws horizon on the frame, by hard coding it to the midway.
	
	
	printf("The dinmension of the input frame is width=%d and height=%d",inputImg.cols, inputImg.rows);
	Point pt1, pt2;
		pt1.x = (int) inputImg.cols/4;
		pt1.y = 0;
		pt2.x = (int) inputImg.cols/4;
		pt2.y = (int) inputImg.rows;
	line(inputImg, pt1, pt2, CV_RGB(0,255,0), 5);
	// cvNamedWindow("DrawHorizon", CV_WINDOW_AUTOSIZE );
	// imshow("DrawHorizon", inputImg);
}

int main( int argc, char** argv )
{  	
	//Images
	cv::Mat inputImg, imgGRAY ;
	cv::Mat outputImg;

	char *videoFileName = 0;
	char *imageFileName = 0;
	cv::VideoCapture video;
	bool useCamera = true; // By Default Input - camera

	bool playMode = false; //By Default, we go framne by frame, video doesnt play continuously
	bool stillImage = false;
	bool verbose = false;
	
	bool rotateFrame = false;
	int rotationAngle = -1;

	int procWidth = -1;
	int procHeight = -1;
	cv::Size procSize;

	//Parse Arguments
	if(argc < 1)
		return -1;
	for(int i = 1; i<argc; i++)
	{
		const char* s = argv[i];

		if(strcmp(s, "-video") == 0){
			
			//Input is a video file
			videoFileName = argv[++i];
			useCamera = false;
		}
		else if(strcmp(s, "-image") == 0){
			//Input is a image file
			imageFileName = argv[++i];
			stillImage = true;
			useCamera = false;
		}
		else if(strcmp(s, "-rotate") == 0){
			rotationAngle = atoi(argv[++i]);
		} 
		else if(strcmp(s, "-resizewidth") == 0) {
			procWidth = atoi(argv[++i]);
		} 
		else {
			perror("ERROR: parameters not recognised\n");
		}
	}

  	// Open video input
	if( useCamera )
		video.open(0);
	else
	{
		if(!stillImage)
			video.open(videoFileName);
	}
  
	//check video input
	int width =0, height = 0, fps = 0, fourcc = 0;
	if(!stillImage){
		if(!video.isOpened()){
			printf("ERROR: can not open camera or video file\n");
			return -1;
		} else {

			// Show video information
			width = (int) video.get(CV_CAP_PROP_FRAME_WIDTH);
			height = (int) video.get(CV_CAP_PROP_FRAME_HEIGHT);
			fps = (int) video.get(CV_CAP_PROP_FPS);
			fourcc = (int) video.get(CV_CAP_PROP_FOURCC);
			
			if(!useCamera)
				printf("Input video: (%d x %d) at %d fps, fourcc = %d\n", width, height, fps, fourcc);
			else
				printf("Input camera: (%d x %d) at %d fps\n", width, height, fps);
		} 
	} else {

		inputImg = cv::imread(imageFileName);
		if(inputImg.empty()) {
			printf("ERROR : Input Image is EMPTY");
			return -1;
		}
			
		
		width = inputImg.cols;
		height = inputImg.rows;
		
		printf("Input image: (%d x %d)\n", width, height);

		playMode = false;

	}

	//resizing logic later on
	if(procWidth != -1)
	{
	
		procHeight = height*((double)procWidth/width);
		procSize = cv::Size(procWidth, procHeight);

		printf("Resize to: (%d x %d)\n", procWidth, procHeight);	
	}
	else
		procSize = cv::Size(width, height);


	int frameNum = 0;
	for( ;; ){
		if(!stillImage)
		{
			printf("\n------------\nFRAME #%6d\n", frameNum);
			frameNum++;

			// Get current image		
			video >> inputImg;
		}

		if(inputImg.empty()) {
			printf("ERROR : Input Image is EMPTY");
			break;
		}

		// Resize to processing size
		cv::resize(inputImg, inputImg, procSize);

		cvNamedWindow("Input", CV_WINDOW_AUTOSIZE );
		imshow("Input", inputImg);


		// ++++++++++++++++++++++++++++++++++++++++
		// Process		
		// ++++++++++++++++++++++++++++++++++++++++
		

		// Color Conversion
		if(inputImg.channels() == 3)
		{
			cv::cvtColor(inputImg, imgGRAY, CV_BGR2GRAY);	
			inputImg.copyTo(outputImg);
		}
		else
		{
			inputImg.copyTo(imgGRAY);
			cv::cvtColor(inputImg, outputImg, CV_GRAY2BGR);
		}


// //to be removed later
// 		cvNamedWindow(window_name, CV_WINDOW_AUTOSIZE );
// 		/// Create a Trackbar for user to enter threshold
//   		createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );	
//   		imgGRAY.copyTo(src_gray);
//   		/// Show the image
//   		CannyThreshold(0,0);
		
		processImage(imgGRAY, outputImg);
		
// to be removed later, doesnt need to be
// in the final code as horizon can be computed with accelerator value
		drawHorizon(outputImg);
		
		// ++++++++++++++++++++++++++++++++++++++++
		// View	
		// ++++++++++++++++++++++++++++++++++++++++
		
	
		cvNamedWindow("Output", CV_WINDOW_AUTOSIZE );
		imshow("Output", outputImg);

		if(playMode)
			{
				cv::waitKey(1);
				char q = (char)waitKey(1);

				if( q == 27 )
				{
					printf("\nStopped by user request\n");
					break;
				}	

			}

		else{
			
			char q = (char)waitKey(1);

				if( q == 27 )
				{
					printf("\nStopped by user request\n");
					break;
				}	
			cv::waitKey(0);
		}
			
			

		if(stillImage)
			break;		

	}

  if(!stillImage)
		video.release();

  return 0;
}




