
#include "StickerColorCalibrator.h"

#include <list>

using namespace cv;
using namespace std;

static double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

static double dist(Point p1, Point p2)
{
	return (double) sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

static void drawShape(Mat image, vector<Point> shapePoints, Scalar color, int thickness)
{
	vector<vector<Point> > contours;
	contours.push_back(shapePoints);

	drawContours(image, contours, 0, color, thickness, 8, vector<Vec4i>(), 0, Point());
}

static bool isContourInsideContour(vector<Point> bigContour, vector<Point> contour)
{
	for (Point pointInContour : contour)
	{
		if (pointPolygonTest(bigContour, pointInContour, false) <= 0)
		{
			return false;
		}
	}
	return true;
}

static void FindStickersInColorChart(Mat& canny, vector<Point>& colorsChart, vector<vector<Point> >& stickers)
{
	// find contours and store them in a list
	vector<vector<Point> > contours;
	findContours(canny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	// filter contours
	vector<Point> approx;
	for (size_t i = 0; i < contours.size(); i++)
	{
		// approximate contour with accuracy proportional
		// to the contour perimeter
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.01, true);

		if (fabs(contourArea(Mat(approx))) < 30)
		{
			continue;
		}

		// check if contour is inside the colorchart
		if (!isContourInsideContour(colorsChart, approx))
		{
			continue;
		}


		stickers.push_back(approx);
	}
}

int StickerColorCalibrator::CalibrateColors(Mat image, size_t minColorsCount, std::vector<ColorChartSticker>& stickersColors)
{
	// define ROI to be at image center
	Rect roi;
	// TODO: add this to config?
	roi.x = image.size().width / 4;
	roi.y = image.size().height / 4;
	roi.width = image.size().width / 2;
	roi.height = image.size().height / 2;

	// focus on center of image only
	Mat crop = image(roi).clone();

	// draw focus area
	vector<Point> croppedRectange(4);
	croppedRectange[0] = roi.tl();
	croppedRectange[1].x = roi.br().x; croppedRectange[1].y = roi.tl().y;
	croppedRectange[2] = roi.br();
	croppedRectange[3].x = roi.tl().x; croppedRectange[3].y = roi.br().y;
	drawShape(image, croppedRectange, Scalar(200, 200, 200), 1);

	// look for color chart in focus area
	vector<Point> colorsChart;
	vector<ColorChartSticker> stickers;
	if (!FindColorsChart(crop, colorsChart, stickers))
	{
		// couldn't detect color chart
		return -1;
	}

	if (minColorsCount > stickers.size())
	{
		return -2;
	}

	// color chart found

	// draw color chart detection
	for (Point& point : colorsChart)
	{
		point.x += roi.x;
		point.y += roi.y;
	}
	drawShape(image, colorsChart, Scalar(255, 0, 0), 2);

	//putText(image, "0", colorsChart[0], 1, 1.2, Scalar(255, 0, 0));
	//putText(image, "1", colorsChart[1], 1, 1.2, Scalar(255, 0, 0));
	//putText(image, "2", colorsChart[2], 1, 1.2, Scalar(255, 0, 0));
	//putText(image, "3", colorsChart[3], 1, 1.2, Scalar(255, 0, 0));

	//imshow("colorchart", image);
	//waitKey();

	// update color charts points coordinations back to original image
	// TODO: maybe this should be handled in FindColorsChart() itself
	for (ColorChartSticker& sticker : stickers)
	{
		for (Point& point : sticker.boundingPoints)
		{
			point.x += roi.x;
			point.y += roi.y;
		}
		sticker.centerPoint.x += roi.x;
		sticker.centerPoint.y += roi.y;
	}
	// draw boundings of detected color chart stickers
	for (int i = 0; i < stickers.size(); i++)
	{

		drawShape(image, stickers[i].boundingPoints, stickers[i].color, 5);

		//string stickerName = to_string(i+1);

		//Size textSize = getTextSize(stickerName, 1, 1.2, 1, nullptr);
		//Point org(stickers[i].centerPoint.x - textSize.width / 2, stickers[i].centerPoint.y + textSize.height / 2);
		//putText(image, stickerName, org, 1, 1.2, Scalar(0, 0, 0), 1);
		//imshow("sticker", image);
		//waitKey();
	}

	// update sticker colors to HSV format
	for (ColorChartSticker& sticker : stickers)
	{
		Mat bgrTemp(1, 1, CV_8UC3, sticker.color);
		Mat hsvTemp(1, 1, CV_8UC3);
		cvtColor(bgrTemp, hsvTemp, COLOR_BGR2HSV);
		Scalar hsvColor = hsvTemp.at<Vec3b>(0, 0);
		sticker.color = hsvColor;
	}

	stickersColors = stickers;
	return 0;
}

static void SortA4SheetCoordinates(vector<Point>& rect)
{
	assert(rect.size() == 4);

	// make sure coordinates are clockwise
	if (contourArea(rect, true) < 0)
	{
		reverse(rect.begin(), rect.end());
	}

	int startingPoint = -1;
	if (dist(rect[1], rect[2]) > dist(rect[1], rect[0]))
	{
		startingPoint = rect[1].x < rect[3].x ? 1 : 3;
	}
	else
	{
		startingPoint = rect[0].x < rect[2].x ? 0 : 2;
	}

	assert(startingPoint != -1);
	
	// rotate elements so that first element is the top left corner of A4 paper
	rotate(rect.begin(), rect.begin() + startingPoint, rect.end());
}

bool StickerColorCalibrator::FindColorsChart(Mat& image, vector<Point>& colorChart, vector<ColorChartSticker>& stickers)
{
	vector<Point> myColorChart;

	Mat gray;
	cvtColor(image, gray, COLOR_BGR2GRAY);

	// blur to enhance edge detection
	Mat blurred(gray);
	// TODO: change kernel size ?
	medianBlur(gray, blurred, 9);

	Mat canny;
	Canny(blurred, canny, 5, 40, 3);

	//imshow("Canny", canny);
	//waitKey();

	// Dilate helps to remove potential holes between edge segments
	Mat dilateImg;
	dilate(canny, dilateImg, Mat());

	//imshow("dilate", dilateImg);
	//waitKey();

	// find contours and store them in a list
	vector<vector<Point> > contours;
	findContours(dilateImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	// filter contours
	bool colorChartFound = false;
	vector<Point> approx;
	for (size_t i = 0; i < contours.size(); i++)
	{
		approx.clear();
		// approximate contour with accuracy proportional to the contour perimeter
		{
			approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.01, true);
		}

		// check if contour form an A4 sheet
		if (!IsA4Sheet(approx))
		{
			continue;
		}

		// A4 sheet found
		// sort coordinates clockwise from top left
		SortA4SheetCoordinates(approx);

		myColorChart = approx;
		colorChartFound = true;
	}

	if (!colorChartFound)
	{
		return false;
	}

	// focus on color chart area
	const double colorChartArea = fabs(contourArea(Mat(myColorChart)));
	Rect colorChartRect = boundingRect(myColorChart);
	Mat colorChartImg = canny(colorChartRect).clone();

	Mat dilateImg2;
	dilate(colorChartImg, dilateImg2, Mat());

	// find contours for stickers
	vector<vector<Point> > contours2;
	vector<Vec4i> hierarchy;
	findContours(dilateImg2, contours2, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, colorChartRect.tl());

	// filter contours
	bool stickersFound = false;
	for (size_t i = 0; i < contours2.size(); i++)
	{
		if (contours2[i].size() < 3)
		{
			continue;
		}
		if (hierarchy[i][3] != -1)
		{
			continue;
		}

		// approximate contour with accuracy proportional
		// to the contour perimeter
		vector<Point> approx;
		approxPolyDP(Mat(contours2[i]), approx, arcLength(Mat(contours2[i]), true)*0.01, true);

		if (fabs(contourArea(Mat(approx))) < colorChartArea / 1000)
		{
			continue;
		}

		// check if contour is inside the colorchart
		if (!isContourInsideContour(myColorChart, approx))
		{
			continue;
		}

		// get center point of detected sticker
		Moments moment = moments((cv::Mat)approx);
		Point stickerCenter((int)(moment.m10 / moment.m00), (int)(moment.m01 / moment.m00));

		//drawShape(image, approx, Scalar(0, 255, 0), 1);
		//imshow("sticker", image);
		//waitKey();


		//bool newSticker = true;
		//for (vector<Point>& sticker : stickers)
		//{
		//	if (pointPolygonTest(sticker, Point(stickerX, stickerY), false) >= 0)
		//	{
		//		newSticker = false;
		//		break;
		//	}
		//}
		//if (!newSticker)
		//{
		//	continue;
		//}

		//drawShape(image, { { stickerX, stickerY } }, Scalar(255, 255, 0), 5);
		//imshow("stickerCenter", image);
		//waitKey();

		// calculate mean color in detected sticker
		Mat mask = Mat::zeros(image.size(), CV_8U);
		drawShape(mask, approx, Scalar(255, 255, 255), 3);
		floodFill(mask, stickerCenter, Scalar(255, 255, 255));
		Mat erodeElement = getStructuringElement(MORPH_RECT, Size(5, 5));
		erode(mask, mask, erodeElement);
		Scalar meanColor = mean(image, mask);

		ColorChartSticker sticker = { approx, stickerCenter, meanColor };
		stickers.push_back(sticker);
		stickersFound = true;
	}
	if (!stickersFound)
	{
		return false;
	}

	// sort stickers from top left to bottom right
	SortStickersInStickerChart(image, myColorChart, stickers);

	//// sort stickers by sortingValue (from top left to bottom right)
	//sort(stickers.begin(), stickers.end(), [](ColorChartSticker& i, ColorChartSticker& j) -> bool
	//{
	//	return i.sortingValue < j.sortingValue;
	//});
	colorChart = myColorChart;

	return true;
}

bool StickerColorCalibrator::IsA4Sheet(const vector<Point>& points)
{
	// check if points form a rectangle
	if (points.size() != 4 ||
		fabs(contourArea(Mat(points))) < 1000 ||
		!isContourConvex(Mat(points)))
	{
		return false;
	}

	// check if points form ~90 degree angles
	double maxCosine = 0;
	const double cosine201 = fabs(angle(points[2], points[0], points[1]));
	const double cosine312 = fabs(angle(points[3], points[1], points[2]));
	const double cosine023 = fabs(angle(points[0], points[2], points[3]));
	maxCosine = MAX(cosine312, cosine201);
	maxCosine = MAX(maxCosine, cosine023);
	if (maxCosine > 0.1)
	{
		return false;
	}

	// check if points forms width / height ratio of ~sqrt(2) (A4 sheet)
	vector<double> distances(3, 0.0);
	distances[0] = dist(points[0], points[1]);
	distances[1] = dist(points[0], points[2]);
	distances[2] = dist(points[0], points[3]);
	sort(distances.begin(), distances.end());

	const double rectangleWidth = distances[0];
	const double rectangleHeight = distances[1];
	const double rectangleDiagonal = distances[2];
	if (abs((rectangleHeight / rectangleWidth) - sqrt(2)) > 0.2 ||
		abs((rectangleDiagonal / rectangleWidth) - sqrt(3)) > 0.2 )
	{
		return false;
	}

	return true;
}

void StickerColorCalibrator::SortStickersInStickerChart(const Mat& image, const vector<Point>& stickersChart, vector<ColorChartSticker>& stickers)
{
	// form a grid of sampling points in the stickersChart
	vector<Point> samplingPoints;

	const int heightPoints = 6;
	const int widthPoints = 10;

	// first, iterate on left and right edges
	LineIterator leftEdgeIt(image, stickersChart[0], stickersChart[3]);
	LineIterator rightEdgeIt(image, stickersChart[1], stickersChart[2]);
	const int leftEdgeSamplingDist = leftEdgeIt.count / (heightPoints+1);
	const int rightEdgeSamplingDist = rightEdgeIt.count / (heightPoints+1);
	vector<Point> leftEdgePoints;
	vector<Point> rightEdgePoints;

	// pick pair of points for grid edges from left and right
	for (int i = 0; i < leftEdgeIt.count && leftEdgePoints.size() < heightPoints; i++, ++leftEdgeIt)
	{
		if (i != 0 && (i % leftEdgeSamplingDist) == 0)
		{
			leftEdgePoints.push_back(leftEdgeIt.pos());
		}
	}
	for (int i = 0; i < rightEdgeIt.count && rightEdgePoints.size() < heightPoints; i++, ++rightEdgeIt)
	{
		if (i != 0 && (i % rightEdgeSamplingDist) == 0)
		{
			rightEdgePoints.push_back(rightEdgeIt.pos());
		}
	}

	// for each grid line from above iterate over it from left to right
	for (int k = 0; k < heightPoints; k++)
	{
		LineIterator widthIt(image, leftEdgePoints[k], rightEdgePoints[k]);
		const int widthSamplingDist = widthIt.count / (widthPoints + 1);

		for (int i = 0, j = 0; i < widthIt.count && j < widthPoints; i++, ++widthIt)
		{
			if (i != 0 && (i % widthSamplingDist) == 0)
			{
				samplingPoints.push_back(widthIt.pos());
				j++;
			}
		}
	}

	// iterate on sampling points, sort stickers by order of sampling points
	vector<ColorChartSticker> remainingStickers = stickers;
	vector<ColorChartSticker> sortedStickers;
	for (vector<Point>::iterator samplingPointsIt = samplingPoints.begin(); samplingPointsIt != samplingPoints.end() && !remainingStickers.empty(); ++samplingPointsIt)
	{
		for (vector<ColorChartSticker>::iterator stickersIt = remainingStickers.begin(); stickersIt != remainingStickers.end(); ++stickersIt)
		{
			if (pointPolygonTest(stickersIt->boundingPoints, *samplingPointsIt, false) >= 0)
			{
				// sampling point is inside sticker
				sortedStickers.push_back(*stickersIt);
				remainingStickers.erase(stickersIt);
				break;
			}
		}
	}

	if (!remainingStickers.empty())
	{
		printf("ERROR: sorting stickers in sticker chart algorithm doesn't have enough sampling points.\n");
	}

	stickers = sortedStickers;


	//Mat& myimage = const_cast<Mat&>(image);
	//for (const Point& point : stickersChart)
	//	circle(myimage, point, 3, Scalar(255, 0, 0), 2);
	//for (Point& point : leftEdgePoints)
	//	circle(myimage, point, 3, Scalar(255, 255, 0), 2);
	//for (Point& point : rightEdgePoints)
	//	circle(myimage, point, 3, Scalar(255, 255, 0), 2);
	//for (Point& point : samplingPoints)
	//	circle(myimage, point, 3, Scalar(0, 255, 255), 2);

	//imshow("myimage", myimage);
	//waitKey();
}