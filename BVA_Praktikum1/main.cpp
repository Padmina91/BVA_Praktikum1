// Code by: Marina Inokuchi
// Please start the .exe from the command line. Pass in 1 parameter, which is the path to the folder in which the three .dcm files are saved.

#include "FileManagement.hpp"
#include "PictureManagement.hpp"

int main(int argc, char** argv) {

    std::string path;
    if (argc == 2) {
        path = argv[1];
    } else {
        std::cout << "Bitte nur ein Argument eingeben." << std::endl;
        exit(-1);
    }

    FileManagement::correctPath(std::ref(path));

    for (int i = 16; i <= 21; i++) {
        // read original picture --------------------------------------------------------------------------------------------------------------------
        std::string histPath = "";
        cv::Mat originalPicture, resultPicture;
        std::string picturePath = path + std::to_string(i) + ".dcm";
        PictureManagement::readDCM(picturePath, std::ref(originalPicture));

        // 1st Filter -------------------------------------------------------------------------------------------------------------------------------
        PictureManagement::filter(std::ref(originalPicture), std::ref(resultPicture), PictureManagement::filterEnum::Gaussian, 13);
        if (i == 20) {
            histPath = path + "gaussianHist" + std::to_string(i) + ".png";
        }
        PictureManagement::segment(std::ref(resultPicture), histPath);

        // 2nd Filter -------------------------------------------------------------------------------------------------------------------------------
        PictureManagement::filter(std::ref(originalPicture), std::ref(resultPicture), PictureManagement::filterEnum::Median, 13);
        if (i == 20) {
            histPath = path + "medianHist" + std::to_string(i) + ".png";
        }
        PictureManagement::segment(std::ref(resultPicture), histPath);

        // 3rd Filter -------------------------------------------------------------------------------------------------------------------------------
        PictureManagement::filter(std::ref(originalPicture), std::ref(resultPicture), PictureManagement::filterEnum::Bilateral);
        if (i == 20) {
            histPath = path + "bilateralHist" + std::to_string(i) + ".png";
        }
        PictureManagement::segment(std::ref(resultPicture), histPath);

        //FileManagement::replaceDataExtension(std::ref(picturePath), ".png");  // von Praktikum 1
        //cv::imwrite(picturePath, picture);                                    // von Praktikum 1
    }

	return 0;
}