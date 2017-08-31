#pragma once

#include "Sticker.h"

#include <vector>

struct ColorChartSticker
{
	std::vector<cv::Point> boundingPoints;
	cv::Point centerPoint;
	cv::Scalar color;
};

class StickerColorCalibrator
{
public:
	int CalibrateColors(cv::Mat image, size_t minColorsCount, std::vector<ColorChartSticker>& stickersColors);

private:
	bool FindColorsChart(cv::Mat& image, std::vector<cv::Point>& colorChart, std::vector<ColorChartSticker>& stickers);
	bool IsA4Sheet(const vector<cv::Point>& points);
	void SortStickersInStickerChart(const cv::Mat& image, const std::vector<cv::Point>& stickersChart, std::vector<ColorChartSticker>& stickers);
};
