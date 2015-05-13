#include "VfxCmd.h"
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFnTransform.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>
#include <maya/MPlug.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include "ErrorCheck.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT " " __FILE__ ": " TOSTRING(__LINE__) 

typedef std::vector<unsigned int>::iterator vecUintIt;
const MString VfxCmd::names[] = { "brow_lower_l", "brow_lower_r",
		"brow_raise_c", "brow_raise_l", "brow_raise_r", "cheek_puff_l",
		"cheek_puff_r", "cheek_raise_l", "cheek_raise_r", "cheek_suck_l",
		"cheek_suck_r", "eye_blink1_l", "eye_blink1_r", "eye_blink2_l",
		"eye_blink2_r", "eye_lidTight_l", "eye_lidTight_r", "eye_shutTight_l",
		"eye_shutTight_r", "eye_upperLidRaise_l", "eye_upperLidRaise_r",
		"jaw_sideways_l", "jaw_sideways_r", "jaw_thrust_c", "mouth_chew_c",
		"mouth_chinRaise_d", "mouth_chinRaise_u", "mouth_dimple_l",
		"mouth_dimple_r", "mouth_funnel_dl", "mouth_funnel_dr",
		"mouth_funnel_ul", "mouth_funnel_ur", "mouth_lipCornerDepressFix_l",
		"mouth_lipCornerDepressFix_r", "mouth_lipCornerDepress_l",
		"mouth_lipCornerDepress_r", "mouth_lipCornerPull_l",
		"mouth_lipCornerPullOpen_l", "mouth_lipCornerPullOpen_r",
		"mouth_lipCornerPull_r", "mouth_lipStretch_l", "mouth_lipStretchOpen_l",
		"mouth_lipStretchOpen_r", "mouth_lipStretch_r",
		"mouth_lowerLipDepress_l", "mouth_lowerLipDepress_r",
		"mouth_lowerLipProtrude_c", "mouth_oh_c", "mouth_oo_c",
		"mouth_pressFix_c", "mouth_press_l", "mouth_press_r", "mouth_pucker_l",
		"mouth_pucker_r", "mouth_screamFix_c", "mouth_sideways_l",
		"mouth_sideways_r", "mouth_stretch_c", "mouth_suck_dl", "mouth_suck_dr",
		"mouth_suck_ul", "mouth_suck_ur", "mouth_upperLipRaise_l",
		"mouth_upperLipRaise_r", "nose_wrinkle_l", "nose_wrinkle_r",
		"smoothCompensated" };

#ifdef _WIN32
#define LOAD_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_6.txt"
#define SAVE_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_out"
#else
#define LOAD_WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights_w2.txt"
#define SAVE_WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights_out"
#endif

MStatus VfxCmd::doIt(const MArgList &args) {

	MStatus stat;

	action = SAVE;
	file_ind = 0;

	// Get state parameter
	MString paramVal;
	unsigned int index;
	index = args.flagIndex("a", "action");
	if (MArgList::kInvalidArgIndex != index) {
		args.get(index + 1, paramVal);
		if (paramVal == "load") {
			action = LOAD;
		}
	}

	index = args.flagIndex("n", "filen");
	if (MArgList::kInvalidArgIndex != index) {
		args.get(index + 1, file_ind);
	}

	// Get all user defined attributes
	MString cmd("getAttr -s shapesBS.weight");
	numWeights = 0;
	MGlobal::executeCommand(cmd, numWeights);

	switch (action) {
	case SAVE: {
		saveWeights();
		return MStatus::kSuccess;
	}
	case LOAD: {
		loadWeights(numWeights);
		return dgMod.doIt();
	}
	}

	return MStatus::kFailure;
}

MStatus VfxCmd::undoIt() {
	MAnimControl anim;
	anim.setMinMaxTime(prevMinTime, prevMaxTime);
	anim.setAnimationStartEndTime(prevStartTime, prevEndTime);
	return dgMod.undoIt();
}

MStatus VfxCmd::redoIt() {
	switch (action) {
	case SAVE: {
		saveWeights();
		return MStatus::kSuccess;
	}
	case LOAD: {
		loadWeights(numWeights);
		return dgMod.doIt();
	}
	}
	return MStatus::kFailure;
}

bool VfxCmd::isUndoable() const {
	return true;
}

void *VfxCmd::creator() {
	return new VfxCmd;
}

void VfxCmd::loadWeights(int numWeights) {
	MString cmd;
	// Read the data from a text file into an array.
	std::fstream myfile( LOAD_WEIGHTS_PATH, std::ios_base::in);
	std::vector<std::vector<float> > weights;
	unsigned int numFrames = 0;
	std::string line;
	unsigned int weightNum = 0;
	std::vector<float> currentWeights(numWeights, 0.0);
	while (std::getline(myfile, line)) {
		// Save current weight
		currentWeights.at(weightNum) = std::stof(line);
		weightNum++;
		if (weightNum == (unsigned int) ((numWeights))) {
			// Save weights for current frame in weights vector
			weights.push_back(currentWeights);
			std::fill(currentWeights.begin(), currentWeights.end(), 0);
			weightNum = 0;
			numFrames++;
		}
	}
	myfile.close();
	// Ignore the last weight
	numWeights--;
	// Break connections.
	for (unsigned int i = 0; i < (unsigned int) (numWeights); i++) {
		cmd = "disconnectAttr shapesBS_";
		cmd = cmd + names[i];
		cmd = cmd + ".output shapesBS.";
		cmd = cmd + names[i];
		dgMod.commandToExecute(cmd);
	}
	cmd = "disconnectAttr con_jaw_c_translateY.output con_jaw_c.translateY";
	dgMod.commandToExecute(cmd);
	// Key everything.
	for (unsigned int i = 0; i < (unsigned int) (numWeights); i++) {
		cmd = "setKeyframe { \"shapesBS.w[";
		cmd = cmd + i;
		cmd = cmd + "]\" }";
		dgMod.commandToExecute(cmd);
	}
	cmd = "setKeyframe  \"con_jaw_c.translateY\"";
	dgMod.commandToExecute(cmd);
	//Save and set max and min time in the playback slider
	MAnimControl anim;
	prevMaxTime = anim.maxTime();
	prevMinTime = anim.minTime();
	prevStartTime = anim.animationStartTime();
	prevEndTime = anim.animationEndTime();
	anim.setMinMaxTime(MTime(1.0), MTime((double) (numFrames)));
	anim.setAnimationStartEndTime(MTime(1.0), MTime((double) (numFrames)));
	//Set numFrames to start at 0 for c++ indexing
	numFrames -= 1;
	for (unsigned int i = 0; i < weights.size(); i++) {
		cmd = "currentTime ";
		cmd = cmd + (i + 1);
		dgMod.commandToExecute(cmd);
		for (unsigned int j = 0; j < weights.at(i).size(); j++) {
			cmd = "setAttr shapesBS.weight[";
			cmd = cmd + j;
			cmd = cmd + "] ";
			cmd = cmd + weights.at(i).at(j);
			dgMod.commandToExecute(cmd);
			cmd = "setKeyframe { \"shapesBS.w[";
			cmd = cmd + j;
			cmd = cmd + "]\" }";
			dgMod.commandToExecute(cmd);
		}
		// Set the teeth movement for this frame, interpolates between the two
		// blend shapes that can open the mouth
		cmd = "setAttr con_jaw_c.translateY ";
		cmd += -3 * (weights.at(i).at(58) + weights.at(i).at(55)) + 1;
		dgMod.commandToExecute(cmd);
		cmd = "setKeyframe  \"con_jaw_c.translateY\"";
		dgMod.commandToExecute(cmd);
	}
}

void VfxCmd::saveWeights() {
	MStatus stat;
	std::string path = SAVE_WEIGHTS_PATH;
	path = path + std::to_string(file_ind) + ".txt";
	std::fstream myfile(path, std::ios_base::app | std::fstream::out);
	MDoubleArray weights;

	MString cmd("getAttr shapesBS.weight");
	stat = MGlobal::executeCommand(cmd, weights);

	MGlobal::displayInfo(MString("Save path: ") + path.c_str());

	for (unsigned int i = 0; i < weights.length(); i++) {
		myfile << weights[i] << endl;
	}

	myfile.close();
}
