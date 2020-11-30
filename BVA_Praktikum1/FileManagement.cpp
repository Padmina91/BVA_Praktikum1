#include "FileManagement.hpp"
#include <iostream>
#include <fstream>

/**
* Edits the path if necessary. Adds a backslash (\) or a forwardslash (/) according to the path formatting.
*/
void FileManagement::correctPath(std::string& path) {
    std::string::reverse_iterator rit = path.rbegin();
    std::string forwardslash = "/";
    std::string backslash = "\\";
    if (!(*rit == '/' || *rit == '\\')) {
        if (path.find(forwardslash) != std::string::npos) {
            path += forwardslash;
        }
        else if (path.find(backslash) != std::string::npos) {
            path += backslash;
        }
        else {
            std::cout << "Es handelt sich nicht um einen validen Pfad. Bitte erneut versuchen." << std::endl;
            exit(-1);
        }
    }
}

/**
* Replaces the former data extension with the new one.
*/
void FileManagement::replaceDataExtension(std::string& picturePath, std::string newFileExtension) {
    size_t pos = picturePath.find_last_of(".");
    picturePath = picturePath.substr(0, pos) + newFileExtension;
}