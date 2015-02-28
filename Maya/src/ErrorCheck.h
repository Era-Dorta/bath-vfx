#ifndef EXTRAFN_H
#define EXTRAFN_H

#include <string.h>
#include <iostream>

#include <maya/MTypes.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

inline void displayError(const char* msg) {
	std::cerr << std::endl << "Error: " << MString(msg);
}

inline bool checkBool(bool result) {
	if (!result)  {
		displayError("bool");
		return true;
	}
	return false;
}

inline bool checkStat(const MStatus& stat, const char* msg) {
	if (stat.error()) {
		displayError(msg);
		return true;
	}
	return false;
}

inline bool checkObject(const MObject& obj, const char* msg) {
	if (obj.isNull()) {
		displayError(msg);
		return true;
	}
	return false;
}

#endif