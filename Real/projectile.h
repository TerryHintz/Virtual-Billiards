//Visual Strategies for Pool Table Game Assistant 
//Leo Zhang		-	301289518	-	cza82@sfu.ca
//Ryan Chahal	-	301290991	-	rschahal@sfu.ca
//Marcus Chan	-	301255529	-	mkc28@sfu.ca

#ifndef PROJECTILE_H
#define PROJECTILE_H
#include <iostream>
#include <vector>
#include <math.h>

#define PI 3.14159265

using namespace std;

int places;
bool found = false;
double distance_adjustment = 1;
// hardest/smallest angle
double bucket1 = 0;
double bucket2 = 0;
double bucket3 = 0;
double bucket4 = 0;
// easiest/largest angle

//Cue Ball Object
class cue {
public:
	int radius;
	int x_pos;
	int y_pos;
};

//Ball Object
class ball
{
public:
	int radius;
	int x_pos;
	int y_pos;
};

//Pocket Object
class hole
{
public:
	const int radius;
	int x_pos;
	int y_pos;
};

struct object {
	double difficult;
	pair<int, int> ballHole;
};

//Used to sort difficulties
struct by_age {
	bool operator()(object const &a, object const &b) const noexcept {
		return a.difficult < b.difficult;
	}
};

//Calculate distance between balls 
bool findEuclid(vector<ball> balls, double x, double y, int self) {
	for (int i = 0; i < balls.size(); i++) {
		//cout << "BALL SIZE " << balls.size() << endl;
		if (balls.size() == 1) {
			//cout << "NEVER" << endl;
			return true;
		}
		if (i == self)
			continue;
		double xDiff = balls[i].x_pos - x;
		double yDiff = balls[i].y_pos - y;
		double distance = pow(pow(xDiff, 2) + pow(yDiff, 2), .5);
		//cout << distance << endl;
		if (distance < 30) {
			return false;
		}
	}
	return true;
}

//Ensures no collision in shot path 
bool noCollision(vector<ball> balls, hole dest, ball target, cue white, int self) {
	double yDiff = white.y_pos - target.y_pos;
	double xDiff = white.x_pos - target.x_pos;
	double slope = yDiff / xDiff;

	if (xDiff < 0)
		slope = -slope;

	for (int i = 0; i < abs((int)xDiff); i++) {
		double x;
		if (xDiff < 0) {
			x = -i + target.x_pos;
		}
		else {
			x = i + target.x_pos;
		}
		double y = slope * i + target.y_pos;
		//double x = i + target.x_pos;
		//cout << x << " " << y << endl;
		bool collide = findEuclid(balls, x, y, self);
		if (!collide)
			return false;
	}

	double yDiff2 = dest.y_pos - target.y_pos;
	double xDiff2 = dest.x_pos - target.x_pos;
	double slope2 = yDiff2 / xDiff2;

	if (xDiff2 < 0)
		slope2 = -slope2;

	for (int i = 0; i < abs((int)xDiff2); i++) {
		double x;
		if (xDiff2 < 0) {
			x = -i + target.x_pos;
		}
		else {
			x = i + target.x_pos;
		}
		double y = slope2 * i + target.y_pos;
		//cout << x << " " << y << endl;
		bool collide = findEuclid(balls, x, y, self);
		if (!collide)
			return false;
	}

	return true;
}

//Learning element distance adjustment
void adjustDistance(double distance, bool made) {
	if (distance > 300) {
		if (made) {
			distance_adjustment -= .1;
			if (distance_adjustment < .5)
				distance_adjustment = .5;
		}
		else {
			distance_adjustment += .1;
			if (distance_adjustment > 1.5)
				distance_adjustment = 1.5;
		}
	}
}

//Learning element angle adjustment 
void adjustAngle(double theta, bool made) {
	if (theta >= 110 && theta < 120)
	{
		if (made)
			bucket1 -= 50;
		else
			bucket1 += 50;
	}
	else if (theta >= 120 && theta < 130)
	{
		if (made)
			bucket2 -= 50;
		else
			bucket2 += 50;
	}
	else if (theta >= 130 && theta < 140)
	{
		if (made)
			bucket3 -= 50;
		else
			bucket3 += 50;
	}
	else if (theta >= 140 && theta < 150)
	{
		if (made)
			bucket4 -= 50;
		else
			bucket4 += 50;
	}
}


//this function will need the cmath library
//Verifies if shot between ball and hole is even possible 
double shot_possible(vector<ball> balls, hole destination, ball location, cue white_ball, int self)
{
	double white2ballD = pow(pow(white_ball.x_pos - location.x_pos, 2) + pow(white_ball.y_pos - location.y_pos, 2), .5);
	double ball2HoleD = pow(pow(destination.x_pos - location.x_pos, 2) + pow(destination.y_pos - location.y_pos, 2), .5);
	double white2HoleD = pow(pow(destination.x_pos - white_ball.x_pos, 2) + pow(destination.y_pos - white_ball.y_pos, 2), .5);
	double theta;

	theta = (pow(ball2HoleD, 2) + pow(white2ballD, 2) - pow(white2HoleD, 2)) / (2 * ball2HoleD*white2ballD);

	theta = theta * 180 / PI;
	theta = 90 - theta;

	double diffi = (.5*white2ballD + 2 * ball2HoleD) * distance_adjustment; //this will be the basic metric for difficulty initially
	//the angle that the white ball would have to hit at will also add a set amount of difficulty.

	//////
	//theta = atan(result2y/result2x) * 180/PI;
	//////
	bool temp = noCollision(balls, destination, location, white_ball, self);
	if (theta < 110)
	{
		return 0;
	}
	else if (!temp) {
		return 0;
	}
	else if (theta >= 110 && theta < 120)
	{
		diffi = diffi + 150 + bucket1;
	}
	else if (theta >= 120 && theta < 130)
	{
		diffi = diffi + 100 + bucket2;
	}
	else if (theta >= 130 && theta < 140)
	{
		diffi = diffi + 50 + bucket3;
	}
	else if (theta >= 140 && theta < 150)
	{
		diffi = diffi + 25 + bucket4;
	}
	//cout << "hole: " << destination.x_pos << " " << destination.y_pos << endl;
	return diffi;
}

//this will return a vector that has the information of the lines that need to be drawn the line angles can be calculated in the main program
//or in another function later on
vector<pair<ball, hole>> shot_display(vector<hole> holes, vector<ball> balls, cue white_ball)
{
	vector<object> hell;
	vector<pair<ball, hole>> result;
	vector<double> storage; // store the difficulty ratings and their place
	vector<pair<int, int>> Places_Pls;  // this will accompany the storages vector to store their places
	double choice[3];
	int count = 0;
	double temps = 0;
	int place[3]; //this variable will be used to keep track of the position of the values we extract
	int large = 0;
	//this for loop is for checking each ball possibilities
	for (int i = 0; i < balls.size(); i++)
	{
		//the for loop below is to check every hole
		for (int j = 0; j < holes.size(); j++)
		{
			//storage.push_back(shot_possible(holes[j], balls[i], white_ball));
			//Places_Pls.push_back(make_pair(j, i));
			double val = shot_possible(balls, holes[j], balls[i], white_ball, i);
			if (val) {
				object temp = { val, make_pair(i, j) };
				hell.push_back(temp);
			}
			//testing
			//cout << "STORAGE " << storage[j] << endl;
			//cout << "PLACES " << Places_Pls[j].first << " " << Places_Pls[j].second << endl;
			//

		}
	}

	//Sort shots vector to put lowest difficulty first
	sort(hell.begin(), hell.end(), by_age());
	for (int i = 0; i < hell.size(); i++) {
		//cout << hell[i].difficult << endl;
	}

	//Returns the 3 best shots
	for (int i = 0; i < 3; i++)
	{
		if (hell.size() <= i)
			break;
		result.push_back(make_pair(balls[hell[i].ballHole.first], holes[hell[i].ballHole.second]));
		cout << hell[i].difficult << endl;
	}
	//after creating that vector it should send the result back to the main program
	return result;
};




#endif