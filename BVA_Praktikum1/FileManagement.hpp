#pragma once

#include <string>

namespace FileManagement {
	void correctPath(std::string& path);
	void replaceDataExtension(std::string& picturePath, std::string newFileExtension);
};