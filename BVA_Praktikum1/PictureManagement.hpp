#pragma once

#include <string>
#include <opencv2/opencv.hpp>

namespace PictureManagement {
	int calculatePictureSize(int fileSize);
	void readDCM(std::string& picturePath, cv::Mat& outPicture);
};