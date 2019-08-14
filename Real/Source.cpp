//Visual Strategies for Pool Table Game Assistant 
//Leo Zhang		-	301289518	-	cza82@sfu.ca
//Ryan Chahal	-	301290991	-	rschahal@sfu.ca
//Marcus Chan	-	301255529	-	mkc28@sfu.ca

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include "projectile.h"
#include <math.h>

#define PI 3.14159265

using namespace cv;
using namespace std;
int main(int argc, char** argv)
{
	cue old_white;
	int selected_shot = 0;
	int ball_count = 99;
	Mat initRGB;
	Mat frame2, frame3;
	vector<pair<ball, hole>> shots;									//this is to hold the result of shot display
	double theta = 0;												//this variable is to hold the angleaway from the shot it is
	double hitx, hity;												//should hold the location where the player should hit the ball
	//Input from webcam
	VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "Failed to take the picuture" << endl;
		return -1;
	}
	cout << "Welcome to the Virtual Pool Game Assistant!" << endl << endl << endl;
	while (1)
	{
		Mat frame;
		cap.read(frame);

		imshow("Camera", frame);

		if (waitKey(30) == 'g') {
			selected_shot = 0;
			cout << "You chose the green shot" << endl;
		}
		if (waitKey(30) == 'y') {
			selected_shot = 1;
			cout << "You chose the yellow shot" << endl;
		}
		if (waitKey(30) == 'r') {
			selected_shot = 2;
			cout << "You chose the red shot" << endl;
		}
		if (waitKey(30) == 's')
		{
			frame2 = frame.clone();
			//imshow("Captured window", frame2);
			imwrite("realTest.png", frame2);


			initRGB = imread("realTest.png");   //from webcam
			cout << "Size: " << initRGB.rows << " rows; " << initRGB.cols << " cols." << endl;
			//imshow("Initial RGB Image", initRGB);


			//Mask Processing
			Mat mask;
			inRange(initRGB, Scalar(65, 100, 65), Scalar(150, 255, 150), mask);		//Determine range of region to be masked (playfield)
			Mat res;
			bitwise_and(initRGB, initRGB, res, mask = mask);						//Apply mask to the default input image, resulting in only playfield
			//imshow("Original", initRGB);
			//imshow("Mask", mask);
			//imshow("Result", res);
			Mat maskInv;
			bitwise_not(mask, maskInv);												//Invert mask to capture everything except playfield										
			//imshow("Mask, inverted", maskInv);
			Mat masked;
			bitwise_and(initRGB, initRGB, masked, maskInv = maskInv);				//Apply inverted mask to default image, resulting in balls
			//imshow("Initial RGB Image, masked", masked);


			//Grayscale, Canny, Hough Processing
			Mat gray;
			cvtColor(masked, gray, COLOR_BGR2GRAY);								//Convert default RGB image to grayscale
			//imshow("Grayscale image", gray);
			medianBlur(gray, gray, 7);											//Blur grayscale image to rid of erroneous details
			Mat cannyEdges;
			Scalar meanG = mean(gray);											//Calcualtions for Canny thresholding
			double minThresh = 0.66*meanG[0];
			double maxThresh = 1.33*meanG[0];
			Canny(gray, cannyEdges, minThresh, maxThresh);						//Run Canny edge detection on the grayscale image 
			//imshow("Canny Edges of Grayscale", cannyEdges);
			vector<Vec3f> circles;
			HoughCircles(cannyEdges, circles, HOUGH_GRADIENT, 1,				//Run Hough circles on the highly simplified Canny Edges image 
				gray.rows / 16,													//Change this value to detect circles with different distances to each other
				200, 19, 10, 29);												//Change the last two parameters (min_radius & max_radius) to detect larger circles


			//Storage of Detected State of Play Data
			vector<ball> balls;
			vector<hole> holes;
			vector<vector<double>> colours;
			vector<vector<double>> solidStripes;
			cue white{ 15, 324, 250 };
			Mat clean = initRGB.clone();
			for (int i = 0; i < circles.size(); i++)
			{
				Vec3i c = circles[i];												//c[0] = x = col; c[1] = y = row; c[2] = radius

				Point center = Point(c[0], c[1]);
				circle(initRGB, center, 1, Scalar(0, 100, 100), 3, LINE_AA);		//Circle center
				int radius = c[2];
				circle(initRGB, center, radius, Scalar(255, 0, 255), 3, LINE_AA);	//Circle outline	

				Vec3b intensity = initRGB.at<Vec3b>(c[1], c[0] - 5);					//BGR Intensity of Each Ball
				uchar Ublue = intensity.val[0];
				uchar Ugreen = intensity.val[1];
				uchar Ured = intensity.val[2];

				int blue = Ublue;
				int green = Ugreen;
				int red = Ured;


				if (c[2] > 16 && blue + green + red > 650) {									//Case of Non-Ball Circles (Holes, Erroneous etc.)
					holes.push_back({ c[2], c[0], c[1] });
				}
				else if (blue + green + red > 700) {										//Case of Cue Ball - largest R + G + B detected (should be ~ (255,255,255))
					white.radius = c[2];
					white.y_pos = c[1];
					white.x_pos = c[0];
				}
				else {																		//Case of Ball Detected 
					Mat circ(clean.rows, clean.cols, CV_8U, Scalar(0));
					circle(circ, center, radius, Scalar(255), -1);							//Mask for ball object 
					//imshow("Circle Mask", circ);
					Mat circRes;
					bitwise_and(clean, clean, circRes, circ = circ);						//Masked image to isolate ball
					//imshow("Individual Ball", circRes);
					double AvB = 0;
					double AvG = 0;
					double AvR = 0;
					double regPixCount = 0;
					double whitePixCount = 0;
					for (int y = 0; y < circRes.rows; y++) {								//Iterate over masked image 
						for (int x = 0; x < circRes.cols; x++) {
							Vec3b circInt = circRes.at<Vec3b>(y, x);						//BGR Intensity of Each Ball
							uchar circBlue = circInt.val[0];
							uchar circGreen = circInt.val[1];
							uchar circRed = circInt.val[2];
							int circBlueI = circBlue;
							int circGreenI = circGreen;
							int circRedI = circRed;
							if (!(circBlueI == 0 && circGreenI == 0 && circRedI == 0)) {	//If pixel is not black (ie if the pixel is within the ball area)
								if (circBlueI + circGreenI + circRedI > 700) {				//If the pixel is white *Needs work* 
									whitePixCount++;
								}
								else {														//If pixel is any colour except white 
									AvB = AvB + circBlueI;
									AvG = AvG + circGreenI;
									AvR = AvR + circRedI;
									regPixCount++;
								}
							}
						}
					}
					double totPix = regPixCount + whitePixCount;							//Sum of all pixels (Ball area ~700 pixels, verified; pi*(15^2)
					//cout << "# White Pixels: " << whitePixCount << "; # Colour Pixels: " << regPixCount << endl;
					double whitePer = 100 * (whitePixCount / totPix);						//Percentage of white pixels 
					AvB = AvB / regPixCount;
					AvG = AvG / regPixCount;
					AvR = AvR / regPixCount;
					//cout << "Average ball BGR values are: Blue = " << AvB << "; Green = " << AvG << "; Red = " << AvR << endl;
					//cout << "White percentage of total area is: " << whitePer << "% " << endl;
					balls.push_back({ c[2], c[0], c[1] });
					colours.push_back({ AvB,AvG,AvR });
					solidStripes.push_back({ regPixCount,whitePixCount,totPix,whitePer });
				}

				//cout << "Detected circle " << i + 1 << ": X = " << c[0] << "; Y = " << c[1] << "; Radius = " << radius << endl;

			}

			//interpolate pockets
			int largeX = 0;
			int smallX = 1000;
			int largeY = 0;
			int smallY = 1000;
			for (int i = 0; i < holes.size(); i++) {
				if (holes[i].x_pos > largeX) {
					largeX = holes[i].x_pos;
				}
				if (holes[i].y_pos > largeY) {
					largeY = holes[i].y_pos;
				}
				if (holes[i].x_pos < smallX) {
					smallX = holes[i].x_pos;
				}
				if (holes[i].y_pos < smallY) {
					smallY = holes[i].y_pos;
				}
			}
			holes.clear();
			holes.push_back({ 25, largeX, largeY });
			holes.push_back({ 25, largeX, smallY });
			holes.push_back({ 25, ((largeX - smallX) / 2) + smallX, largeY });
			holes.push_back({ 25, ((largeX - smallX) / 2) + smallX, smallY });
			holes.push_back({ 25, smallX, largeY });
			holes.push_back({ 25, smallX, smallY });

			cout << largeX << " " << smallX << " " << largeY << " " << smallY << endl;

			for (int i = 0; i < balls.size(); i++) {
				if (balls[i].x_pos > largeX || balls[i].x_pos < smallX || balls[i].y_pos > largeY || balls[i].y_pos < smallY) {
					balls.erase(balls.begin() + i);
					i--;
				}
			}


			cout << endl << "Cue Ball Details:" << endl;
			cout << "X = " << white.x_pos << "; Y =  " << white.y_pos << "; Radius = " << white.radius << endl << endl;

			if (ball_count != 99) {
				ball selected_ball = shots[selected_shot].first;
				hole selected_hole = shots[selected_shot].second;

				//
				double white2ballD = pow(pow(old_white.x_pos - selected_ball.x_pos, 2) + pow(old_white.y_pos - selected_ball.y_pos, 2), .5);
				double ball2HoleD = pow(pow(selected_hole.x_pos - selected_ball.x_pos, 2) + pow(selected_hole.y_pos - selected_ball.y_pos, 2), .5);
				double white2HoleD = pow(pow(selected_hole.x_pos - old_white.x_pos, 2) + pow(selected_hole.y_pos - old_white.y_pos, 2), .5);
				double theta;

				theta = (pow(ball2HoleD, 2) + pow(white2ballD, 2) - pow(white2HoleD, 2)) / (2 * ball2HoleD*white2ballD);

				theta = theta * 180 / PI;
				theta = 90 - theta;

				double distance = white2ballD + ball2HoleD;

				if (ball_count > balls.size()) {
					cout << "Shot made!" << endl;
					adjustDistance(distance, true);
					adjustAngle(theta, true);
				}
				else {
					cout << "Shot missed!" << endl;
					adjustDistance(distance, false);
					adjustAngle(theta, false);
				}
			}
			old_white = white;

			ball_count = balls.size();

			cout << "Regular Ball Details:" << endl;
			for (int i = 0; i < balls.size(); i++) {
				cout << "X = " << balls[i].x_pos << "; Y =  " << balls[i].y_pos << "; Radius = " << balls[i].radius << endl;
				cout << "Blue = " << colours[i][0] << "; Green =  " << colours[i][1] << "; Red = " << colours[i][2] << endl;
				cout << "# Colour Pixels = " << solidStripes[i][0] << "; # White Pixels = " << solidStripes[i][1]
					<< "; # Total Pixels = " << solidStripes[i][2] << "; % White = " << solidStripes[i][3] << endl << endl;
			}

			cout << "Pocket Hole Details:" << endl;
			for (int i = 0; i < holes.size(); i++) {
				cout << "X = " << holes[i].x_pos << "; Y =  " << holes[i].y_pos << "; Radius = " << holes[i].radius << endl << endl;
			}
			//gets all the data we need in shots
			shots = shot_display(holes, balls, white);

			//variables below are to keep track of the hit equation
			//////////////////////
			int x1, x2;
			int y1, y2;

			//////////////////////
			for (int i = 0; i < 3; i++)
			{
				if (shots.size() <= i)
					break;
				//balls
				x1 = shots[i].first.x_pos;
				y1 = shots[i].first.y_pos;
				//holes
				x2 = shots[i].second.x_pos;
				y2 = shots[i].second.y_pos;

				Scalar color = Scalar(255, 0, 0);
				if (i == 0) {
					color = Scalar(0, 204, 0);
				}
				else if (i == 1) {
					color = Scalar(51, 255, 255);
				}
				else {
					color = Scalar(0, 0, 255);
				}

				double diffX = x1 - x2; //pos
				double diffY = y1 - y2; //pos
				double fracX = diffX / (abs(diffX) + abs(diffY));
				double fracY = diffY / (abs(diffX) + abs(diffY));

				line(initRGB, Point(white.x_pos, white.y_pos), Point(x1 + (fracX * 30), y1 + (fracY * 30)), color, 2);
				line(initRGB, Point(x1, y1), Point(x2, y2), color, 2);
				circle(initRGB, Point(x1 + (fracX * 30), y1 + (fracY * 30)), 13, Scalar(255, 255, 255), 2);
			}

			//now we have to draw the lines between variable white and the point located by the hit variables
			//also if we want from the ball(x1,y1) to the hole (x2, y2)
			imshow("Detected Balls on Initial RGB Image", initRGB);
			waitKey();
		}

		if (waitKey(30) == 'd')
		{
			break;
		}
	}

	return 0;
}