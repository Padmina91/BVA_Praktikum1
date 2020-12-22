#pragma once

#include <string>
#include <opencv2/opencv.hpp>

namespace PictureManagement {
	enum class filterEnum {Gaussian, Median, Bilateral};

	int calculatePictureSize(int fileSize);
	void readDCM(std::string& picturePath, cv::Mat& outPicture);
	void filter(cv::Mat& inPicture, cv::Mat& outPicture, filterEnum whichFilter, int filterSize = 0);
	void drawHistogram(cv::Mat& inPicture, cv::Mat& outHistogram, cv::Mat& pictureWithHistogram, int width, int height);
	void segment(cv::Mat& inPicture, std::string histPath = "");
};