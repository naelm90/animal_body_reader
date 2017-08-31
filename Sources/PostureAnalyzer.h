#pragma once

#include "Body.h"
#include "..\BBLib\BBVector2D.h"

enum HeadDir { HEAD_DIR_UNKNOWN, HEAD_DIR_CENTER, HEAD_DIR_LEFT, HEAD_DIR_RIGHT };
enum HeadPos { HEAD_POS_UNKNOWN, HEAD_POS_STRAIGHT, HEAD_POS_UP, HEAD_POS_DOWN };
enum TailDir { TAIL_DIR_UNKNOWN, TAIL_DIR_CENTER, TAIL_DIR_LEFT, TAIL_DIR_RIGHT, TAIL_DIR_DOWN };
enum BodyPos { BODY_POS_UNKNOWN, BODY_POS_STANDING, BODY_POS_SITTING, BODY_POS_LYING };

struct Posture
{
	bool isDetected;
	HeadDir headDir;
	HeadPos headPos;
	TailDir tailDir;
	BodyPos bodyPos;
	double  backNeckDist;
	double  backHeadDist;
};

class PostureAnalyzer
{
public:

	const int minHeadAngle = 5;  // below this considered straight
	const int minTailAngle = 15; // below this considered straight

	PostureAnalyzer();
	int CalculateBodyPosture(const Body &body);
	void GetPosture(Posture &postureOut) { postureOut = _posture; }
	void GetPostureText(const Posture &posture, char *text);
	void SetPixelsInCm(double pixInCm) { _pixelsInCm = pixInCm; }

private:
	Posture _posture;
	double _pixelsInCm;

	// the use of 2D vector library is as follows:
	// north <- x = width, east <- y = height
	// this way we get angles w.r.t. frame's x axis
	CBBPolarVector2D _bodyVector;
	CBBPolarVector2D _tailVector;
	CBBPolarVector2D _headVector;
};