#include "PictureManagement.hpp"
#include <fstream>
#include <cmath>

/**
* Calculates the picture size of a .dcm file in bytes.
* Prerequisites:
* - picture height = picture width
* - picture size = 2^n
*/
int PictureManagement::calculatePictureSize(int fileSize) {
    int i = 1;
    while (static_cast<int>(pow(2, i + 1) * 2) < fileSize) {
        i++;
    }
    return static_cast<int>(pow(2, i) * 2);
}

/**
* Reads a .dcm file, converts it to a cv::Mat object and shows the result on the screen.
*/
void PictureManagement::readDCM(std::string& picturePath, cv::Mat& outPicture) {
	std::ifstream dcmFile(picturePath, std::ifstream::binary);
    if (dcmFile) {
        dcmFile.seekg(0, dcmFile.end);
        int fileSize = dcmFile.tellg(); // size in bytes
        int pictureSize = calculatePictureSize(fileSize); // size in bytes
        int widthInPixel = sqrt(pictureSize / 2); // width = height, so only one size is needed
        int offset = fileSize - pictureSize;
        dcmFile.seekg(offset, dcmFile.beg); // skip header
        char* buffer = new char[pictureSize];
        dcmFile.read(buffer, pictureSize);
        cv::Mat picture(widthInPixel, widthInPixel, CV_16UC1, buffer);
        double minVal, maxVal;
        cv::minMaxLoc(picture, &minVal, &maxVal);
        double alpha = 255.0 / maxVal;
        cv::convertScaleAbs(picture, outPicture, alpha, 0.0);
        delete[] buffer;
        if (!outPicture.empty()) {
            cv::imshow("Ergebnis", outPicture);
            cv::waitKey(0);
        }
    }
    else {
        std::cout << "Die Dateien befinden sich nicht an dem angegebenen Pfad. Bitte erneut versuchen." << std::endl;
        exit(-1);
    }
}