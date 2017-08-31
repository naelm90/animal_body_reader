#include "PostureAnalyzer.h"
#include "..\BBLib\BBTypes.h"

PostureAnalyzer::PostureAnalyzer()
{
	_posture.headDir = HEAD_DIR_UNKNOWN;
	_posture.headPos = HEAD_POS_UNKNOWN;
	_posture.tailDir = TAIL_DIR_UNKNOWN;
	_posture.bodyPos = BODY_POS_UNKNOWN;
}


int PostureAnalyzer::CalculateBodyPosture(const Body &body)
{
	_posture.isDetected = false;
	_posture.backNeckDist = -1.0;
	_posture.backHeadDist = -1.0;

	// if we don't see the sticker on the back - there is not enough information
	if (!body.back.IsVisible()) {
		return 1;
	}

	// if we see neither head nor neck - - there is not enough information
	if (!body.head.IsVisible() && !body.neck.IsVisible()) {
		return 2;
	}

	_posture.isDetected = true;

	// deduce head position
	if (body.head.IsVisible() && body.neck.IsVisible()) {
		_posture.headPos = HEAD_POS_STRAIGHT;
	}
	if (body.head.IsVisible() && !body.neck.IsVisible()) {
		_posture.headPos = HEAD_POS_UP;
	}
	if (!body.head.IsVisible() && body.neck.IsVisible()) {
		_posture.headPos = HEAD_POS_DOWN;
	}

	// calculate body orientation
	if (body.neck.IsVisible()) {
		_bodyVector = CBBCartVector2D(body.neck.pObject->xPos - body.back.pObject->xPos,
			                          body.neck.pObject->yPos - body.back.pObject->yPos);
		_posture.backNeckDist = _bodyVector.GetMagnitude() / _pixelsInCm;
		// commented out because doesn't really work...
		//if (_posture.backNeckDist < 20.0) { 
		//	_posture.bodyPos = BODY_POS_SITTING;
		//}
		//else {
		//	_posture.bodyPos = BODY_POS_UNKNOWN;
		//}
	}
	else {
		_bodyVector = CBBCartVector2D(body.head.pObject->xPos - body.back.pObject->xPos,
			                          body.head.pObject->yPos - body.back.pObject->yPos);
		_posture.backHeadDist = _bodyVector.GetMagnitude() / _pixelsInCm;
		// commented out because doesn't really work...
		//if (_posture.backHeadDist < 23.0) {
		//	_posture.bodyPos = BODY_POS_SITTING;
		//}
		//else {
		//	_posture.bodyPos = BODY_POS_UNKNOWN;
		//}
	} 
	
	int bodyDir = int(_bodyVector.GetDirection360());
	//printf("Body: %d  ", bodyDir);

	// calculate tail orientation if tail is visible
	if (body.tail.IsVisible()) {
		_tailVector = CBBCartVector2D(body.back.pObject->xPos - body.tail.pObject->xPos,
			                          body.back.pObject->yPos - body.tail.pObject->yPos);
		int tailDir = int(_tailVector.GetDirection360());
		//printf("Tail: %d (", tailDir);
		int dirDiff = abs(bodyDir - tailDir) % 360;
		if (dirDiff < minTailAngle) {
			_posture.tailDir = TAIL_DIR_CENTER;
			//printf("straight)");
		}
		else {
			double tailCrossProduct = _bodyVector.Cross(_tailVector);
			if (tailCrossProduct < 0.0) {
				_posture.tailDir = TAIL_DIR_RIGHT;
				//printf("right)");
			}
			else {
				_posture.tailDir = TAIL_DIR_LEFT;
				//printf("left)");
			}
		}
	}
	else {
		_posture.tailDir = TAIL_DIR_DOWN;
	}
	
	// calculate head orientation if head and neck are visible
	if (body.head.IsVisible() && body.neck.IsVisible()) {
		_headVector = CBBCartVector2D(body.head.pObject->xPos - body.neck.pObject->xPos,
			                          body.head.pObject->yPos - body.neck.pObject->yPos);
		int headDir = int(_headVector.GetDirection360());
		//printf("  Head: %d (", headDir);
		int dirDiff = abs(bodyDir - headDir) % 360;
		if (dirDiff < minHeadAngle) {
			_posture.headDir = HEAD_DIR_CENTER;
			//printf("straight)");
		}
		else {
			double headCrossProduct = _headVector.Cross(_bodyVector);
			if (headCrossProduct < 0.0) {
				_posture.headDir = HEAD_DIR_RIGHT;
				//printf("right)");
			}
			else {
				_posture.headDir = HEAD_DIR_LEFT;
				//printf("left)");
			}
		}
	}
	else {
		_posture.headDir = HEAD_DIR_UNKNOWN;
	}

	//printf("\n");
	return NO_ERROR;
}

void PostureAnalyzer::GetPostureText(const Posture &posture, char *text)
{
	char bodyPosText[15];
	char headPosText[15];
	char headDirText[15];
	char tailDirText[15];
	char bodyDistText[50];

	if (!posture.isDetected) {
		sprintf(text, "Posture not detected");
		return;
	}

	switch (posture.bodyPos) {
		case BODY_POS_STANDING: sprintf(bodyPosText, "Standing"); break;
		case BODY_POS_SITTING: sprintf(bodyPosText, "Sitting"); break;
		case BODY_POS_LYING: sprintf(bodyPosText, "Lying"); break;
		default: sprintf(bodyPosText, "Unknown");
	}

	switch (posture.headPos) {
		case HEAD_POS_STRAIGHT: sprintf(headPosText, "Straight"); break;
		case HEAD_POS_UP: sprintf(headPosText, "Up"); break;
		case HEAD_POS_DOWN: sprintf(headPosText, "Down"); break;
		default: sprintf(headPosText, "Unknown");
	}

	switch (posture.headDir) {
		case HEAD_DIR_CENTER: sprintf(headDirText, "Center"); break;
		case HEAD_DIR_LEFT: sprintf(headDirText, "Left"); break;
		case HEAD_DIR_RIGHT: sprintf(headDirText, "Right"); break;
		default: sprintf(headDirText, "Unknown");
	}

	switch (posture.tailDir) {
		case TAIL_DIR_CENTER: sprintf(tailDirText, "Center"); break;
		case TAIL_DIR_LEFT: sprintf(tailDirText, "Left"); break;
		case TAIL_DIR_RIGHT: sprintf(tailDirText, "Right"); break;
		case TAIL_DIR_DOWN: sprintf(tailDirText, "Down"); break;
		default: sprintf(tailDirText, "Unknown");
	}

	if (posture.backNeckDist != -1.0) {
		sprintf(bodyDistText, "dist to neck: %.1fcm", posture.backNeckDist);
	}
	else if (posture.backHeadDist != -1.0) {
		sprintf(bodyDistText, "dist to head: %.1fcm", posture.backHeadDist);
	}
	else {
		sprintf(bodyDistText, "dist not available");
	}

	sprintf(text, "Body pos: %s %s | Head tilt: %s | Head dir: %s | Tail dir: %s",
		bodyPosText, bodyDistText, headPosText, headDirText, tailDirText);
}