#pragma once

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>

using namespace std;

struct Object
{
	int xPos;
	int yPos;
	int area;
	string type; // typically, for sticker's color
	string label; // typically, for body part
	cv::Scalar color;
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
};

struct Sticker
{
	Sticker() {	strcpy(name, ""); }
	char name[20]; // usually represents sticker's color
	cv::Scalar minHSV; // color lower boundaries in HSV
	cv::Scalar maxHSV;  // color upper boundaries in HSV
	unsigned int size; // size in units of minimal object's area
	unsigned int qty; // how many stickers of this type are used
	vector<Object> objects; // qty objects, associated with the sticker
};
