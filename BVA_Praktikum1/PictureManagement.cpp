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
        ushort* buffer = new ushort[pictureSize/2];
        dcmFile.read((char*)buffer, pictureSize);
        cv::Mat picture(widthInPixel, widthInPixel, CV_16UC1, buffer);
        double minVal, maxVal;
        cv::minMaxLoc(picture, &minVal, &maxVal);
        double alpha = 255.0 / (maxVal - minVal);
        double beta = -minVal * alpha;
        cv::convertScaleAbs(picture, outPicture, alpha, beta);
        delete[] buffer;
        if (!outPicture.empty()) {
            cv::imshow("Original", outPicture);
            cv::waitKey(0);
        }
    } else {
        std::cout << "Die Dateien befinden sich nicht an dem angegebenen Pfad. Bitte erneut versuchen." << std::endl;
        exit(-1);
    }
}

/**
* Filters the input picture (cv::Mat object) according to the selected filter and filter size. For the bilateral filter, no filter size is needed.
* The result picture is then shown on the screen.
*/
void PictureManagement::filter(cv::Mat& inPicture, cv::Mat& outPicture, filterEnum whichFilter, int filterSize) {
    if ((whichFilter == filterEnum::Gaussian || whichFilter == filterEnum::Median)
        && (filterSize % 2 != 1 || filterSize < 3)) {
        std::cout << "Filter kann nicht angewendet werden, da filterSize nicht gerade und nicht kleiner als 3 sein darf." << std::endl;
        return;
    }
    if (whichFilter == filterEnum::Gaussian) {
        std::cout << "Gauss-Filter anwenden..." << std::endl;
        double sigma = 0.3 * ((filterSize - 1) * 0.5 - 1) + 0.8;
        cv::GaussianBlur(inPicture, outPicture, cv::Size(filterSize, filterSize), sigma);
        cv::imshow("Ergebnisbild nach Gauss-Filter", outPicture);
        cv::waitKey(0);
    } else if (whichFilter == filterEnum::Median) {
        std::cout << "Median-Filter anwenden..." << std::endl;
        cv::medianBlur(inPicture, outPicture, filterSize);
        cv::imshow("Ergebnisbild nach Median-Filter", outPicture);
        cv::waitKey(0);
    } else {
        std::cout << "Bilateral-Filter anwenden..." << std::endl;
        cv::bilateralFilter(inPicture, outPicture, 5, 50, 50);
        cv::imshow("Ergebnisbild nach Bilateral-Filter", outPicture);
        cv::waitKey(0);
    }
}

void PictureManagement::drawHistogram(cv::Mat& inPicture, cv::Mat& outHistogram, cv::Mat& histogramPicture, int histWidth, int histHeight) {
    int histSize = histWidth;
    float range[] = {0, 256}; // the upper boundary is exclusive
    const float* histRange = {range};
    cv::Mat blurredPicture;
    cv::blur(inPicture, blurredPicture, cv::Size(7, 7)); // first, use the boxcar filter with a 7x7 pixel kernel size
    //cv::calcHist(&inPicture, 1, 0, cv::Mat(), outHistogram, 1, &histSize, &histRange);
    cv::calcHist(&blurredPicture, 1, 0, cv::Mat(), outHistogram, 1, &histSize, &histRange);
    std::cout << "in drawHistogram: outHistogram rows: " << outHistogram.rows << std::endl;
    std::cout << "in drawHistogram: outHistogram: " << outHistogram << ", ";
    histogramPicture = cv::Mat(histHeight, histWidth, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::normalize(outHistogram, outHistogram, 0, histogramPicture.rows, cv::NORM_MINMAX, -1, cv::Mat());
    int bin_w = cvRound((double) histWidth / histSize);
    for (int i = 1; i < histSize; i++) {
        cv::line(histogramPicture, cv::Point(bin_w * (i - 1), histHeight - cvRound(outHistogram.at<float>(i - 1))),
            cv::Point(bin_w * (i), histHeight - cvRound(outHistogram.at<float>(i))),
            cv::Scalar(255, 0, 0), 2, 8, 0);
    }
    cv::imshow("Histrogramm", histogramPicture);
    cv::waitKey(0);
}


void PictureManagement::segment(cv::Mat& inPicture, std::string histPath) {
    double minVal, maxVal;
    cv::minMaxLoc(inPicture, &minVal, &maxVal);

    std::cout << "minVal: " << minVal << ", ";
    std::cout << "maxVal: " << maxVal << ", ";
    int histWidth = maxVal;
    std::cout << "histWidth: " << histWidth << ".\n";
    cv::Mat histogram, histogramPicture;
    drawHistogram(std::ref(inPicture), std::ref(histogram), std::ref(histogramPicture), histWidth, 400);
    if (!histPath.empty()) {
        cv::imwrite(histPath, histogramPicture);
    }
    std::cout << "Histrogram rows: " << histogram.rows << std::endl;

    //std::cout << histogram << ", ";
}