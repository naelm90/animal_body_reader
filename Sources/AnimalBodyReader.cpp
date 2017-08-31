#include "AnimalBodyReader.h"
#include "..\BBLib\BBWinUtils.h"
#include "..\BBLib\BBUnitsConv.h"
#include <list>
#include <algorithm>
#include <math.h>

#include <iostream>
#include <cstring>

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event != EVENT_LBUTTONDOWN)
	{
		return;
	}

	assert(nullptr != userdata);
	AnimalBodyReader* reader = (AnimalBodyReader*)userdata;
	reader->MouseClickAt(x, y);
}

void DrawTextAt(const std::string& text, Mat image, int x, int y)
{
	Size textSize = getTextSize(text, FONT_HERSHEY_PLAIN, 1.2, 1, nullptr);

	Point org(x - textSize.width / 2, y + textSize.height / 2);

	putText(image, text, org, FONT_HERSHEY_PLAIN, 1.2, Scalar(255, 255, 255), 3);
	putText(image, text, org, FONT_HERSHEY_PLAIN, 1.2, Scalar(0, 0, 0), 1);
}
void DrawTextInBottomCenter(const std::string& text, Mat image)
{
	const Size textSize = getTextSize(text, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr);

	Point org;
	org.x = (int)((image.size().width - textSize.width) / 2);
	org.y = (int)((image.size().height / 5) * 4);

	putText(image, text, org, FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 6);
	putText(image, text, org, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 2);
}
void DrawTextInBottomLeft(const std::string& text, Mat image)
{
	const Size textSize = getTextSize(text, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr);

	Point org;
	org.x = 10;
	org.y = (int)(image.size().height - textSize.height - 10);

	putText(image, text, org, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 6);
	putText(image, text, org, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 2);
}

AnimalBodyReader::AnimalBodyReader() : _colorsCalibrated(true), _startBodyDetection(true)
{
	strcpy(_body.head.name, "Head");
	strcpy(_body.neck.name, "Neck");
	strcpy(_body.back.name, "Back");
	strcpy(_body.tail.name, "Tail");
	strcpy(_body.legs[LEG_FRONT_LEFT ].name, "LegFL");
	strcpy(_body.legs[LEG_FRONT_RIGHT].name, "LegFR");
	strcpy(_body.legs[LEG_BACK_LEFT  ].name, "LegBL");
	strcpy(_body.legs[LEG_BACK_RIGHT ].name, "LegBR");
	_lastUsedFrameTime = 0;
	_frameCounter = 0;

	// read screen resolution
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &_desktopResolution);
	// now the top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (_desktopResolution.right, _desktopResolution.bottom)

	_mouseCallBackEnabled = false;
}

int AnimalBodyReader::StartVideo()
{
	if (_config.readFromLiveStream) {
		if (!_capture.open(_config.deviceId)) {
			return 1;
		}
		// set height and width of capture frame, as well as codec
		_capture.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
		_capture.set(CV_CAP_PROP_FRAME_WIDTH, _config.frameWidth);
		_capture.set(CV_CAP_PROP_FRAME_HEIGHT, _config.frameHeight);
	}
	else {
		if (!_capture.open(_config.filename)) {
			return 2;
		}
	}

	// get height and width of capture frame
	_frameWidth = (int)_capture.get(CV_CAP_PROP_FRAME_WIDTH);
	_frameHeight = (int)_capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	_fps = (int)_capture.get(CV_CAP_PROP_FPS);
	
	if (_config.readFromLiveStream) {
		printf("Starting live video from device ID=%d\n", _config.deviceId);
		printf("Video frame size: %d x %d\n", _frameWidth, _frameHeight);
	}
	else {
		printf("Starting video from file: %s\n", _config.filename);
		printf("Video frame size: %d x %d, FPS: %d\n", _frameWidth, _frameHeight, _fps);
	}

	// use camera height and HFOV to convert pixels to cm
	double horViewInCm = tan(BB_DEG_2_RAD(_config.HFOV / 2.0)) * _config.height * 2.0;
	_postureAnalyzer.SetPixelsInCm(_frameWidth / horViewInCm);

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	waitKey(500);
	return 0; // no error
}

int AnimalBodyReader::HandleVideoFrame()
{
	static Mat dst, detected_edges, src, src_gray;

	clock_t now = clock();
	int msSinceLastUsedFrame = int(double(now - _lastUsedFrameTime) / CLOCKS_PER_SEC * 1000.0);

	// store image to matrix
	if (!_capture.read(_cameraFeed)) {
		if (_config.readFromLiveStream) {
			printf("Error reading live video stream\n");
			return 1;
		}
		else {
			destroyWindow("Output");
			printf("Finished playing video file\n");
			return 2;
		}
	}


	namedWindow("Output");

	//if (17 > _frameCounter)
	//{
	//	_frameCounter++;
	//	return 0;
	//}

	//imshow("Raw frame", _cameraFeed);
	//waitKey();

	char timeStr[30];
	BBUtGetTimeStr(timeStr, 30, ':', TRUE);

	bool waitForConfirmation = false;
	vector<ColorChartSticker> detectedStickers;


	if (msSinceLastUsedFrame >= _config.frameInterval)
	{
		src = _cameraFeed;
		if (!src.data) return 3;
		_lastUsedFrameTime = clock();

		// calibrate sticker colors if needed
		if (!_colorsCalibrated)
		{
			int stickersColorChartDetected = _stickerColorCalibrator.CalibrateColors(_cameraFeed, _stickers.size(), detectedStickers);

			if (stickersColorChartDetected == -1)
			{
				printf("Frame%05u: %s %s\n", _frameCounter, timeStr, "Couldn't detect stickers color chart - skipping frame");
			}
			else if (stickersColorChartDetected == -2)
			{
				printf("Frame%05u: %s %s\n", _frameCounter, timeStr, "Stickers color chart detected with fewer colors than configuration - skipping frame");
			}
			else
			{
				printf("Frame%05u: %s %s\n", _frameCounter, timeStr, "Stickers color chart detected - waiting for confirmation...");
				waitForConfirmation = true;
			}
		}
		else if (_startBodyDetection)
		{
			//convert frame from BGR to HSV colorspace
			cvtColor(_cameraFeed, _HSV, COLOR_BGR2HSV);

			// search for stickers
			for (unsigned int i = 0; i < _stickers.size(); i++) {
				//for (unsigned int i = 3; i < 4; i++) {
				if (_stickers[i].minHSV[0] <= _stickers[i].maxHSV[0]) {
					inRange(_HSV, _stickers[i].minHSV, _stickers[i].maxHSV, _threshold1);
				}
				else {
					// red -> two ranges
					Scalar minHSV, maxHSV;
					// lower range
					minHSV = _stickers[i].minHSV; minHSV[0] = 0;
					maxHSV = _stickers[i].maxHSV;
					inRange(_HSV, minHSV, maxHSV, _threshold1);
					// upper range
					minHSV = _stickers[i].minHSV;
					maxHSV = _stickers[i].maxHSV; maxHSV[0] = 179;
					inRange(_HSV, minHSV, maxHSV, _threshold2);
					// combine the two ranges
					_threshold1 |= _threshold2;
				}

				MorphOps(_threshold1);

				FindObjectsForSticker(i);
			}

			GetBodyPositionFromStickers();

			_postureAnalyzer.CalculateBodyPosture(_body);
			_postureAnalyzer.GetPosture(_posture);
			char postureText[200];
			_postureAnalyzer.GetPostureText(_posture, postureText);

			printf("Frame%05u: %s %s\n", _frameCounter, timeStr, postureText);

			DrawBodyLines();
			for (unsigned int i = 0; i < _stickers.size(); i++) {
				DrawSticker(i);
			}
		}
	}

	_frameCounter++;

	// ALL USER INTERACTIONS MUST BE HANDLED IN THIS FUNCTION ONLY !!!
	return HandleUserInteraction(waitForConfirmation, detectedStickers);
}

int AnimalBodyReader::HandleUserInteraction(bool waitForConfirmation, const vector<ColorChartSticker>& detectedStickers)
{
	// delay so that screen can refresh.
	// image will not appear without this waitKey() command
	// 27 is the ASCII code of ESC (exit)
	// 32 is the ASCII code of Space (pause / unpause)
	// 8 is the ASCII code of Backscpace (reject calibration)
	// 13 is the ASCII code of Enter/Carriage Return (confirm calibration)
	// 67 is the ASCII code for "C" character (initiate calibration)
	static const int ESC_KEY = 27;
	static const int SPACE_KEY = 32;
	static const int BACKSPACE_KEY = 8;
	static const int ENTER_KEY = 13;
	static const int CAPITAL_C_KEY = 67;
	static const int LOWER_C_KEY = 99;

	static int delayMs = _fps > 0 ? 1000 / _fps : 40;

	if (waitForConfirmation)
	{
		// waiting for user to confirm stickers colors detection

		//draw edge to let user know we are paused
		DrawScreenEdge();

		// extract HSV ranges from deteted colors
		vector<pair<Scalar, Scalar> > detectedRanges;
		int c1, c2;
		detectedRanges = CalculateDetectedMinMaxHSV(detectedStickers, c1, c2);
		if (c1 != 0)
		{
			// detected ranges are overlapping
			DrawTextInBottomCenter("Color #" + to_string(c1) + " and color #" + to_string(c2) + " are overlapping! Press 'Backspace' to retry...", _cameraFeed);

			// name stickers by color numbers
			for (int i = 0; i < detectedStickers.size(); i++)
			{
				string stickerName = to_string(i + 1);
				DrawTextAt(stickerName, _cameraFeed, detectedStickers[i].centerPoint.x, detectedStickers[i].centerPoint.y);
			}

			imshow("Output", _cameraFeed);

			while (true)
			{
				int key = waitKey();
				if (key == ESC_KEY) return 12345;
				if (key == BACKSPACE_KEY) return 0;
			}
		}

		// detected ranges are good, move on to naming the colors from user

		// handling colors naming will be done in mouse callback function

		Mat cameraFeedClone = _cameraFeed.clone();
		DrawTextInBottomCenter("Please click on '" + string(_stickers[0].name) + "' color or press 'Backspace' to cancel...", _cameraFeed);

		// prepare all info for mouse callback to handle
		MouseCallbackData mouseCallBackData;
		mouseCallBackData.image = cameraFeedClone;
		mouseCallBackData.focusAreaStartX = _cameraFeed.size().width / 4;
		mouseCallBackData.focusAreaStartY = _cameraFeed.size().height / 4;
		mouseCallBackData.focusAreaEndX = mouseCallBackData.focusAreaStartX + (_cameraFeed.size().width / 2);
		mouseCallBackData.focusAreaEndY = mouseCallBackData.focusAreaStartY + (_cameraFeed.size().height / 2);
		mouseCallBackData.configuredStickers = _stickers;
		mouseCallBackData.detectedStickers = detectedStickers;
		_callbackData = mouseCallBackData;

		// enable mouse callback
		_mouseCallBackEnabled = true;
		// TODO: is setting callback multiple times safe ?
		setMouseCallback("Output", CallBackFunc, this);
		imshow("Output", _cameraFeed);

		while (true)
		{
			int key = waitKey();
			if (key == ESC_KEY || key == BACKSPACE_KEY || key == ENTER_KEY)
			{
				// user stop, disable callback and wait for it to finish
				_mouseCallBackEnabled = false;
				std::lock_guard<std::mutex> lock(_mtx);
			}
			if (key == ESC_KEY) return 12345;
			if (key == BACKSPACE_KEY) return 0;
			if (key == ENTER_KEY)
			{
				// user done
				if (_callbackData.configuredStickers.size() != _callbackData.configuredStickersColors.size())
				{
					_mouseCallBackEnabled = true;
					continue;
				}

				assert(_callbackData.configuredStickers.size() == _stickers.size());

				for (int i = 0; i < _stickers.size(); i++)
				{
					pair<Scalar, Scalar> minmax = CalculateMinMaxHSVForColor(_callbackData.configuredStickersColors.at(i));
					_stickers[i].minHSV = minmax.first;
					_stickers[i].maxHSV = minmax.second;

					printf("Confirmed sticker%d %10s minH=\"%03d\" maxH=\"%03d\" minS=\"%03d\" maxS=\"%03d\" minV=\"%03d\" maxV=\"%03d\"\n", i + 1,
						_stickers[i].name,
						(int)_stickers[i].minHSV[0], (int)_stickers[i].maxHSV[0],
						(int)_stickers[i].minHSV[1], (int)_stickers[i].maxHSV[1],
						(int)_stickers[i].minHSV[2], (int)_stickers[i].maxHSV[2]);
				}

				_colorsCalibrated = true;

				return 0;
			}
		}
	}


	if (!_colorsCalibrated)
	{
		DrawTextInBottomCenter("Looking for stickers color chart at center of frame...", _cameraFeed);
	}
	if (_colorsCalibrated && !_startBodyDetection)
	{
		DrawTextInBottomCenter("Press 'Enter' to start agents detection...", _cameraFeed);
	}
	if (_colorsCalibrated && _startBodyDetection)
	{
		DrawTextInBottomLeft("Press 'C' for color calibration...", _cameraFeed);
	}

	imshow("Output", _cameraFeed);
	int key1 = waitKey(delayMs);
	if (key1 == ESC_KEY) return 12345;
	if (key1 == ENTER_KEY && !_startBodyDetection)
	{
		_startBodyDetection = true;
		return 0;
	}
	if (key1 == CAPITAL_C_KEY || key1 == LOWER_C_KEY)
	{
		_colorsCalibrated = false;
		_startBodyDetection = false;
		return 0;
	}
	if (key1 == SPACE_KEY)
	{
		char timeStr[30];
		BBUtGetTimeStr(timeStr, 30, ':', TRUE);
		printf("Frame%05u: %s Paused.\n", _frameCounter - 1, timeStr);
		while (true) {
			DrawScreenEdge();
			imshow("Output", _cameraFeed);
			int key2 = waitKey(delayMs);
			if (key2 == ESC_KEY) return 12345;
			if (key2 == SPACE_KEY) break;
		}
	}

	return 0;
}

void AnimalBodyReader::FindObjectsForSticker(int stickerIndex)
{
	// clear sticker's objects from the previously-processed frame
	_stickers[stickerIndex].objects.clear();

	list<Object> objects;
	Mat temp;
	_threshold1.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	if (hierarchy.size() > 0) {
		int numObjects = (int)hierarchy.size();
		//if number of objects greater than maxObjects we assume we have a noisy filter
		if (numObjects < maxObjects) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				int area = int(moment.m00);

				//if the area is less than minimum then it is probably just noise
				//if the area is bigger than maximum, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > _config.minPixels && area < _config.maxPixels * int(_stickers[stickerIndex].size)) 	{

					Object object;
					object.type = string(_stickers[stickerIndex].name);
					object.area = area;
					//printf("%d\n", area);
					object.xPos = int(moment.m10 / area);
					object.yPos = int(moment.m01 / area);
					if (object.xPos < _frameWidth && object.yPos < _frameHeight) {
						object.color = _cameraFeed.at<Vec3b>(Point(object.xPos, object.yPos));
					}
					else {
						// for some reason once in a while it happens...
						object.color = Vec3b(100, 100, 100);
					}
					object.contours.push_back(contours[index]);
					object.hierarchy.push_back(hierarchy[index]);
					objects.push_back(object);
				}
			} 

			// the following code finds up to qty (config) biggest objects for the given sticker,
			// and these objects are added tio the sticker in the sorted order
			while (objects.size() > 0 && _stickers[stickerIndex].objects.size() < _stickers[stickerIndex].qty) {
				int biggestObjArea = 0;
				std::list<Object>::iterator biggestObjIt, it;
				for (it = objects.begin(); it != objects.end(); it++)
				{
					if (it->area > biggestObjArea) {
						biggestObjIt = it;
						biggestObjArea = it->area;
					}
				}
				_stickers[stickerIndex].objects.push_back(*biggestObjIt);
				objects.erase(biggestObjIt);
			}
		}
		else {
			putText(_cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
		}
	}
}

static bool isOverlappingHue(int minH1, int maxH1, int minH2, int maxH2)
{
	if (minH1 <= maxH1)
	{
		if ((minH2 >= minH1 && minH2 <= maxH1) || (maxH2 >= minH1 && maxH2 <= maxH1))
		{
			return true;
		}
	}
	else
	{
		if ((minH2 >= 0 && minH1 <= maxH1) || (maxH2 >= 0 && maxH2 <= maxH1) ||
			(minH2 >= minH1 && minH1 <= 179) || (maxH2 >= minH1 && maxH2 <= 179))
		{
			return true;
		}

	}

	return false;
}

pair<Scalar, Scalar> AnimalBodyReader::CalculateMinMaxHSVForColor(Scalar color) const
{
	const int range = _config.calibrationRange;

	const int hue = (int)color[0];
	const int sat = (int)color[1];
	const int val = (int)color[2];

	assert(hue >= 0 && hue < 180);

	Scalar minHSV;
	minHSV[0] = ((hue - range) < 0) ? (hue - range + 180) : (hue - range);
	minHSV[1] = (sat > 150) ? 150 : ((sat > 20) ? (sat - 20) : sat);
	minHSV[2] = (val > 150) ? 150 : ((val > 20) ? (val - 20) : val);

	Scalar maxHSV;
	maxHSV[0] = (hue + range) >= 180 ? (hue + range - 180) : (hue + range);
	maxHSV[1] = 255;
	maxHSV[2] = 255;

	return make_pair(minHSV, maxHSV);
}

vector<pair<Scalar, Scalar> > AnimalBodyReader::CalculateDetectedMinMaxHSV(vector<ColorChartSticker> stickersColors, int& c1, int& c2) const
{
	vector<pair<Scalar, Scalar> > minMaxValues;

	for (unsigned int i = 0; i < stickersColors.size(); i++)
	{

		pair<Scalar, Scalar> minmax = CalculateMinMaxHSVForColor(stickersColors[i].color);
		Scalar minHSV = minmax.first;
		Scalar maxHSV = minmax.second;
		minMaxValues.push_back(minmax);

		printf("Detected color #%d: minH=\"%03d\" maxH=\"%03d\" minS=\"%03d\" maxS=\"%03d\" minV=\"%03d\" maxV=\"%03d\"\n", i+1,
			(int)minHSV[0], (int)maxHSV[0],
			(int)minHSV[1], (int)maxHSV[1],
			(int)minHSV[2], (int)maxHSV[2]);
	}

	c1 = 0;
	c2 = 0;

	// check for overlapping
	bool isOverlapping = false;
	for (int i = 0; i < minMaxValues.size() && !isOverlapping; i++)
	{
		int minH1 = (int) minMaxValues[i].first[0];
		int maxH1 = (int) minMaxValues[i].second[0];

		for (int j = 0; j < minMaxValues.size() && !isOverlapping; j++)
		{
			if (i == j) continue;

			int minH2 = (int)minMaxValues[j].first[0];
			int maxH2 = (int)minMaxValues[j].second[0];

			if (isOverlappingHue(minH1, maxH1, minH2, maxH2))
			{
				printf("ERROR: Color #%d and #%d are overlapping. Please re-configure range or use different stickers.\n", i+1, j+1);
				c1 = i + 1;
				c2 = j + 1;
				isOverlapping = true;
			}
		}
	}

	return minMaxValues;
}

void AnimalBodyReader::DrawScreenEdge()
{
	Rect rec;
	rec.x = 0;
	rec.y = 0;
	rec.width = _cameraFeed.size().width;
	rec.height = _cameraFeed.size().height;

	rectangle(_cameraFeed, rec, Scalar(0, 0, 255), 15, 8);
}

void AnimalBodyReader::DrawObject(Object &obj)
{
	cv::drawContours(_cameraFeed, obj.contours, 0, obj.color, 2, 8, obj.hierarchy);
	cv::circle(_cameraFeed, cv::Point(obj.xPos, obj.yPos), 5, obj.color);
	//cv::putText(_cameraFeed, "(" + to_string(obj.xPos) + "," + to_string(obj.yPos) + ")", cv::Point(obj.xPos + 40, obj.yPos + 10), 1, 1, obj.color);
	if (obj.label == "") {
		cv::putText(_cameraFeed, obj.type, cv::Point(obj.xPos + 30, obj.yPos + 5), 1, 1.2, obj.color);
	}
	else {
		cv::putText(_cameraFeed, obj.label, cv::Point(obj.xPos + 30, obj.yPos + 5), 1, 1.2, obj.color, 2);
	}
}


void AnimalBodyReader::DrawSticker(int stickerIndex)
{
	for (unsigned int i = 0; i < _stickers[stickerIndex].objects.size(); i++)
	{
		DrawObject(_stickers[stickerIndex].objects[i]);
	}
}


void AnimalBodyReader::DrawBodyLines()
{
	if (_body.back.IsVisible() && _body.neck.IsVisible()) {
		cv::line(_cameraFeed, 
			Point(_body.neck.pObject->xPos, _body.neck.pObject->yPos),
			Point(_body.back.pObject->xPos, _body.back.pObject->yPos),
			_body.back.pObject->color, 1);
	}
	else if (_body.back.IsVisible() && _body.head.IsVisible()) {
		// if neck is not visible but head is then draw line from back to head
		cv::line(_cameraFeed,
			Point(_body.head.pObject->xPos, _body.head.pObject->yPos),
			Point(_body.back.pObject->xPos, _body.back.pObject->yPos),
			_body.back.pObject->color, 1);
	}
	if (_body.neck.IsVisible() && _body.head.IsVisible()) {
		cv::line(_cameraFeed,
			Point(_body.head.pObject->xPos, _body.head.pObject->yPos),
			Point(_body.neck.pObject->xPos, _body.neck.pObject->yPos),
			_body.head.pObject->color, 1);
	}
	if (_body.tail.IsVisible() && _body.back.IsVisible()) {
		cv::line(_cameraFeed,
			Point(_body.tail.pObject->xPos, _body.tail.pObject->yPos),
			Point(_body.back.pObject->xPos, _body.back.pObject->yPos),
			_body.tail.pObject->color, 1);
	}
}


void AnimalBodyReader::MorphOps(Mat &thresh)
{
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	//erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	//dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}


void AnimalBodyReader::GetBodyPositionFromStickers()
{
	// for head, neck, back and tail - straightforward as we only have one sticker
	if (_body.head.pSticker->objects.size() > 0) {
		_body.head.pObject = &_body.head.pSticker->objects[0];
		_body.head.pObject->label = string(_body.head.name);
	}
	else {
		_body.head.pObject = NULL;
	}

	if (_body.neck.pSticker->objects.size() > 0) {
		_body.neck.pObject = &_body.neck.pSticker->objects[0];
		_body.neck.pObject->label = string(_body.neck.name);
	}
	else {
		_body.neck.pObject = NULL;
	}

	if (_body.back.pSticker->objects.size() > 0) {
		_body.back.pObject = &_body.back.pSticker->objects[0];
		_body.back.pObject->label = string(_body.back.name);
	}
	else {
		_body.back.pObject = NULL;
	}

	if (_body.tail.pSticker->objects.size() > 0) {
		_body.tail.pObject = &_body.tail.pSticker->objects[0];
		_body.tail.pObject->label = string(_body.tail.name);
	}
	else {
		_body.tail.pObject = NULL;
	}

	// TODO: read leg positions
}


int AnimalBodyReader::ReadConfig(string filename)
{
	int nRc = 0, i, valInt;
	char valStr[50] = "";
	//double valdouble;
	//int valInt;

	// initialize XML parser
	if (_xmlParser.Init() != NO_ERROR) {
		printf(_FUNC_ "ERROR: Failed to initialize XML Parser\n");
		return 1;
	}

	TCHAR currentDir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, currentDir);
	string configFileName = currentDir;
	configFileName += "\\" + filename;
	printf("Reading config from: '%s'\n", configFileName.c_str());

	if (_xmlParser.LoadDocFromFile(configFileName.c_str()) != NO_ERROR) {
		printf(_FUNC_ "ERROR: Failed to load document from file %s\n", filename.c_str());
		return 2;
	}

	// read general parameters
	if (_xmlParser.GetAttributeString("General/@input", valStr) == NO_ERROR) {
		if (!strcmp(valStr, "Live") || !strcmp(valStr, "live")) {
			_config.readFromLiveStream = true;
		}
		else if (!strcmp(valStr, "File") || !strcmp(valStr, "file")) {
			_config.readFromLiveStream = false;
		}
		else {
			printf(_FUNC_ "Error: Could not determine input type, should be Live or File\n");
			return 3;
		}
	}
	else {
		return 3;
	}
	
	if (_config.readFromLiveStream) {
		if (_xmlParser.GetAttributeInt("General/Live/@deviceId", _config.deviceId) != NO_ERROR) {
			return 3;
		}
	}
	else {
		if (_xmlParser.GetAttributeString("General/File/@name", _config.filename) != NO_ERROR) {
			return 3;
		}
	}
	if (_xmlParser.GetAttributeInt("General/Camera/@frameWidth", _config.frameWidth) != NO_ERROR)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeInt("General/Camera/@frameHeight", _config.frameHeight) != NO_ERROR)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeInt("General/Camera/@height", _config.height) != NO_ERROR || _config.height < 100)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeDouble("General/Camera/@HFOV", _config.HFOV) != NO_ERROR || _config.HFOV < 1.0)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeInt("General/Object/@minPixels", _config.minPixels) != NO_ERROR || _config.minPixels < 1)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeInt("General/Object/@maxPixels", _config.maxPixels) != NO_ERROR ||  _config.minPixels < 1)
	{
		return 3;
	}
	if (_xmlParser.GetAttributeInt("General/Algorithm/@frameInterval", _config.frameInterval) != NO_ERROR || _config.frameInterval < 1)
	{
		return 3;
	}

	// read stickers colors mode
	if (_xmlParser.GetAttributeString("Stickers/Calibration/@enabled", valStr) == NO_ERROR) {
		if (!strcmp(valStr, "False") || !strcmp(valStr, "false")) {
			_config.calibrateStickersColors = false;
			_colorsCalibrated = true;
			_startBodyDetection = true;
		}
		else if (!strcmp(valStr, "True") || !strcmp(valStr, "true")) {
			_config.calibrateStickersColors = true;
			_colorsCalibrated = false;
			_startBodyDetection = false;
		}
		else {
			printf(_FUNC_ "Error: Could not determine stickers color calibration mode\n");
			return 4;
		}
	}
	else {
		return 4;
	}
	if (_xmlParser.GetAttributeInt("Stickers/Calibration/@range", _config.calibrationRange) != NO_ERROR)
	{
		return 4;
	}
		
	// read stickers
	char stickerElem[50];
	for (i = 0; ; i++) {
		sprintf(stickerElem, "Stickers/Sticker%d/@name", i + 1);
		if (_xmlParser.GetAttributeString(stickerElem, valStr, false) == NO_ERROR) {
			_stickers.push_back(Sticker());
			strcpy(_stickers[i].name, valStr);
		}
		else {
			break;
		}

		if (!_config.calibrateStickersColors)
		{
			sprintf(stickerElem, "Stickers/Sticker%d/@minH", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].minHSV[0] = (double)valInt;
			}
			else {
				return 4;
			}
			sprintf(stickerElem, "Stickers/Sticker%d/@maxH", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].maxHSV[0] = (double)valInt;
			}
			else {
				return 4;
			}
			sprintf(stickerElem, "Stickers/Sticker%d/@minS", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].minHSV[1] = (double)valInt;
			}
			else {
				return 4;
			}
			sprintf(stickerElem, "Stickers/Sticker%d/@maxS", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].maxHSV[1] = (double)valInt;
			}
			else {
				return 4;
			}
			sprintf(stickerElem, "Stickers/Sticker%d/@minV", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].minHSV[2] = (double)valInt;
			}
			else {
				return 4;
			}
			sprintf(stickerElem, "Stickers/Sticker%d/@maxV", i + 1);
			if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0 && valInt < 256) {
				_stickers[i].maxHSV[2] = (double)valInt;
			}
			else {
				return 4;
			}
		}
		sprintf(stickerElem, "Stickers/Sticker%d/@size", i + 1);
		if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt > 0) {
			_stickers[i].size = valInt;
		}
		else {
			return 4;
		}
		sprintf(stickerElem, "Stickers/Sticker%d/@qty", i + 1);
		if (_xmlParser.GetAttributeInt(stickerElem, valInt) == NO_ERROR && valInt >= 0) {
			_stickers[i].qty = valInt;
		}
		else {
			return 4;
		}
	}

	// read body parts and their respective stickers
	if (_xmlParser.GetAttributeString("BodyParts/Head/@sticker", valStr) != NO_ERROR ||
		!(_body.head.pSticker = GetStickerPtrByName(valStr)))  {
		printf(_FUNC_ "Sticker for head is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/Neck/@sticker", valStr) != NO_ERROR ||
		!(_body.neck.pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for neck is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/Back/@sticker", valStr) != NO_ERROR ||
		!(_body.back.pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for back is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/Tail/@sticker", valStr) != NO_ERROR ||
		!(_body.tail.pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for tail is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/LegFL/@sticker", valStr) != NO_ERROR ||
		!(_body.legs[LEG_FRONT_LEFT].pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for front left leg is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/LegFR/@sticker", valStr) != NO_ERROR ||
		!(_body.legs[LEG_FRONT_RIGHT].pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for front right leg is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/LegBL/@sticker", valStr) != NO_ERROR ||
		!(_body.legs[LEG_BACK_LEFT].pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for back left leg is not properly defined!\n");
		return 5;
	}
	if (_xmlParser.GetAttributeString("BodyParts/LegBR/@sticker", valStr) != NO_ERROR ||
		!(_body.legs[LEG_BACK_RIGHT].pSticker = GetStickerPtrByName(valStr))) {
		printf(_FUNC_ "Sticker for back right leg is not properly defined!\n");
		return 5;
	}

	return NO_ERROR;
}


Sticker *AnimalBodyReader::GetStickerPtrByName(char *name)
{
	for (unsigned int i = 0; i < _stickers.size(); i++)
	{
		if (!strcmp(name, _stickers[i].name)) {
			return &_stickers[i];
		}
	}
	return NULL;
}

void AnimalBodyReader::MouseClickAt(int x, int y)
{
	std::lock_guard<std::mutex> lock(_mtx);

	if (!_mouseCallBackEnabled)
	{
		return;
	}

	if (_callbackData.configuredStickersColors.size() == _callbackData.configuredStickers.size())
	{
		return;
	}

	// ignore non-focus area
	if (x < _callbackData.focusAreaStartX || x > _callbackData.focusAreaEndX ||
		y < _callbackData.focusAreaStartY || y > _callbackData.focusAreaEndY)
	{
		return;
	}

	const int currentConfigSticker = (int) _callbackData.configuredStickersColors.size();
	Sticker configSticker = _callbackData.configuredStickers[currentConfigSticker];

	for (int i = 0; i < _callbackData.detectedStickers.size(); i++)
	{
		if (pointPolygonTest(_callbackData.detectedStickers[i].boundingPoints, Point(x, y), false) >= 0)
		{
			// click is inside detected sticker

			// store color for configured sticker
			_callbackData.configuredStickersColors[currentConfigSticker] = _callbackData.detectedStickers[i].color;

			// update output with configured sticker color name
			DrawTextAt(configSticker.name, _callbackData.image, _callbackData.detectedStickers[i].centerPoint.x, _callbackData.detectedStickers[i].centerPoint.y);
			printf("Registered sticker color '%s'\n", configSticker.name);

			// remove this detected stickers from future detected stickers
			_callbackData.detectedStickers.erase(_callbackData.detectedStickers.begin() + i);

			// prepare for next configured sticker
			if (_callbackData.configuredStickersColors.size() != _callbackData.configuredStickers.size())
			{
				Mat tempImage = _callbackData.image.clone();
				DrawTextInBottomCenter("Please click on '" + string(_callbackData.configuredStickers[currentConfigSticker + 1].name) + "' color or press 'Backspace' to cancel...", tempImage);
				imshow("Output", tempImage);
			}
			else
			{
				Mat tempImage = _callbackData.image.clone();
				DrawTextInBottomCenter("Press 'Enter' to confirm or 'Backspace' to cancel...", tempImage);
				imshow("Output", tempImage);
			}

			break;
		}
	}
}