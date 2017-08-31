#pragma once

#include "..\BBLib\BBWinXmlParser.h"
#include <vector>
#include <map>
#include <ctime>
#include <mutex>

#include "PostureAnalyzer.h"
#include "StickerColorCalibrator.h"

using namespace std;
using namespace cv;

class AnimalBodyReader
{
public:

	const int maxObjects = 50; // we shouldnt' have more objects of the sticker color

	struct Config
	{
		bool readFromLiveStream; // true - from camera, false - from file
		int deviceId;
		char filename[200];
		int frameWidth;
		int frameHeight;
		int height; // in cm, above dog's average height
		double HFOV; // in degrees
		int minPixels;
		int maxPixels;
		int frameInterval;
		bool calibrateStickersColors;
		int calibrationRange;
	};

	AnimalBodyReader();

	int ReadConfig(string filename);
	int StartVideo();
	int HandleVideoFrame();

	void MouseClickAt(int x, int y);

private:
	Config _config;
	vector<Sticker> _stickers; // defined stickers
	Body _body;
	Posture _posture;
	PostureAnalyzer _postureAnalyzer;
	StickerColorCalibrator _stickerColorCalibrator;
	bool _colorsCalibrated;
	bool _startBodyDetection;
   
	VideoCapture _capture; //video capture object to acquire webcam feed
	Mat _cameraFeed; // matrix to store each frame of the webcam feed
	Mat _threshold1, _threshold2;
	Mat _HSV;
	RECT _desktopResolution;

	// video properties
	int _frameWidth;
	int _frameHeight;
	int _fps;
	int _frameCounter;
	
	clock_t _lastUsedFrameTime;

	CBBWinXmlParser _xmlParser;

	mutex _mtx;
	volatile bool _mouseCallBackEnabled;

	struct MouseCallbackData
	{
		Mat image;

		int focusAreaStartX;
		int focusAreaStartY;
		int focusAreaEndX;
		int focusAreaEndY;

		vector<Sticker> configuredStickers;
		vector<ColorChartSticker> detectedStickers;

		map<int, Scalar> configuredStickersColors;
	};
	MouseCallbackData _callbackData;


	// auxiliary methods

	int HandleUserInteraction(bool waitForConfirmation, const vector<ColorChartSticker>& detectedStickers);

	// calculate detected color ranges and return overlapping ranges if they exist
	pair<Scalar, Scalar> CalculateMinMaxHSVForColor(Scalar color) const;
	vector<pair<Scalar, Scalar> > CalculateDetectedMinMaxHSV(vector<ColorChartSticker> stickersColors, int& c1, int& c2) const;

	void MorphOps(Mat &thresh);
	void FindObjectsForSticker(int stickerIndex);
	void GetBodyPositionFromStickers();

	void DrawScreenEdge();
	void DrawSticker(int stickerIndex);
	void DrawObject(Object &obj);
	void DrawBodyLines();

	Sticker *GetStickerPtrByName(char *name); // returns NULL if sticker with given name not found
};