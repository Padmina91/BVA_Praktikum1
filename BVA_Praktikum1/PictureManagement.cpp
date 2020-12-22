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
    float range[] = {0, histWidth}; // the upper boundary is exclusive
    const float* histRange = {range};
    cv::Mat blurredPicture, histogramToDisplay;
    cv::blur(inPicture, blurredPicture, cv::Size(7, 7)); // first, use the boxcar filter with a 7x7 pixel kernel
    cv::calcHist(&blurredPicture, 1, 0, cv::Mat(), outHistogram, 1, &histSize, &histRange);
    cv::normalize(outHistogram, outHistogram, 0, 1, cv::NormTypes::NORM_MINMAX, -1); // normalize between 0 and 1 for further usage
    cv::normalize(outHistogram, histogramToDisplay, 0, histHeight, cv::NormTypes::NORM_MINMAX, -1); // normalize between 0 and 400 for displaying the histogram

    int binWidth = (int)((double) histWidth / histSize);
    for (int i = 1; i < histSize; i++) {
        cv::line(histogramPicture,
            cv::Point(binWidth * (i - 1), histHeight - cvRound(histogramToDisplay.at<float>(i - 1))),
            cv::Point(binWidth * i, histHeight - cvRound(histogramToDisplay.at<float>(i))),
            cv::Scalar(255, 0, 255), 2);
    }
    cv::imshow("Histrogramm", histogramPicture);
    cv::waitKey(0);
}


void PictureManagement::segment(cv::Mat& inPicture, std::string histPath) {
    double minVal, maxVal;
    const int histHeight = 400;
    cv::minMaxLoc(inPicture, &minVal, &maxVal);
    const int histWidth = maxVal;
    cv::Mat histogram = cv::Mat(histWidth, 1, CV_32F, cv::Scalar::all(0));
    cv::Mat histogramPicture = cv::Mat(histHeight, histWidth, CV_8UC3, cv::Scalar::all(0));
    drawHistogram(std::ref(inPicture), std::ref(histogram), std::ref(histogramPicture), histWidth, histHeight);

    // Algorithmus von Tsai --------------------------------------------------------------------------------------------------------------------
    const int r = 25;
    std::vector<double> angleChangesVec;
    // initialize angleChangesVec
    for (int i = 0; i < histWidth; i++) {
        angleChangesVec.emplace_back(0.0);
    }
    for (int t = r; t < (histWidth - r); t++) {
        double sum = 0;
        for (int i = 1; i <= r; i++) {
            sum += (static_cast<double>(histogram.at<float>(t + i)) - static_cast<double>(histogram.at<float>(t - i)))/ (2.0 * i);
        }
        angleChangesVec[t] = 1.0 / r * sum;
    }
    cv::Mat angleChangesMat = cv::Mat(angleChangesVec);

    std::vector<double> curvaturesVec;
    // initialize curvaturesVec
    for (int i = 0; i < histWidth; i++) {
        curvaturesVec.emplace_back(0.0);
    }
    for (int t = r; t < (histWidth - r); t++) {
        double sum = 0;
        for (int i = 1; i <= r; i++) {
            sum += abs(static_cast<double>(angleChangesMat.at<double>(t + i)) - static_cast<double>(angleChangesMat.at<double>(t - i)));
        }
        curvaturesVec[t] = 1.0 / r * sum;
    }
    cv::Mat curvaturesMat = cv::Mat(curvaturesVec); // all the values are positive because of abs()
    cv::normalize(curvaturesMat, curvaturesMat, 0, histHeight, cv::NormTypes::NORM_MINMAX, -1); // normalize between 0 and 400 for displaying the curvature values.
    for (int i = 1; i < histWidth; i++) {
        cv::line(histogramPicture,
            cv::Point(i - 1, histHeight - cvRound(curvaturesMat.at<double>(i - 1))),
            cv::Point(i, histHeight - cvRound(curvaturesMat.at<double>(i))),
            cv::Scalar(255, 255, 0), 2);
    }

    cv::imshow("Histrogramm mit Kruemmung", histogramPicture);
    cv::waitKey(0);
    if (!histPath.empty()) {
        cv::imwrite(histPath, histogramPicture);
    }
}