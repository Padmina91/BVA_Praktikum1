// Code by: Marina Inokuchi
// Please start the .exe from the command line. Pass in 1 parameter, which is the path to the folder in which the three .dcm files are saved.

#include "FileManagement.hpp"
#include "PictureManagement.hpp"

int main(int argc, char** argv) {

    std::string path;
    if (argc == 2) {
        path = argv[1];
    }
    else {
        std::cout << "Bitte nur ein Argument eingeben." << std::endl;
        exit(-1);
    }

    FileManagement::correctPath(std::ref(path));

    for (int i = 0; i < 3; i++) {
        cv::Mat picture;
        std::string picturePath = path + std::to_string(i) + ".dcm";
        PictureManagement::readDCM(picturePath, std::ref(picture));
        FileManagement::replaceDataExtension(std::ref(picturePath), ".png");
        cv::imwrite(picturePath, picture);
    }

	return 0;
}