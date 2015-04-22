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
		"mouth_upperLipRaise_r", "nose_wrinkle_l", "nose_wrinkle_r" };

#ifdef OS_WINDOWS
#define WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_6.txt"
#else
#define WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights_6.txt"
#endif

MStatus VfxCmd::doIt(const MArgList &args) {

	MStatus stat;

	// Get all user defined attributes
	MString cmd("getAttr -s shapesBS.weight");
	int numWeights = 0;
	MGlobal::executeCommand(cmd, numWeights);
	numWeights -= 1;

	// Read the data from a text file into an array.
	std::fstream myfile( WEIGHTS_PATH, std::ios_base::in);
	std::vector<std::vector<float>> weights;
	float a;
	unsigned int numFrames = 2;

	weights.clear();
	weights.resize(numFrames, std::vector<float>(numWeights, 0.0));
	for (unsigned int i = 0; i < numFrames; i++) {
		for (unsigned int j = 0; j < (unsigned int) numWeights; j++) {
			myfile >> a;
			weights.at(i).at(j) = a;
		}
	}
	myfile.close();

	// Break connections.
	for (unsigned int j = 0; j < (unsigned int) numWeights; j++) {
		cmd = "disconnectAttr shapesBS_";
		cmd = cmd + names[j];
		cmd = cmd + ".output shapesBS.";
		cmd = cmd + names[j];
		stat = dgMod.commandToExecute(cmd);
		if (stat == MS::kFailure) {
			break;
		}
	}

	// Key everything.
	for (unsigned int j = 0; j < (unsigned int) numWeights; j++) {
		cmd = "setKeyframe { \"shapesBS.w[";
		cmd = cmd + j;
		cmd = cmd + "]\" }";
		dgMod.commandToExecute(cmd);
	}

	// Set frame number in Maya.
	cmd = "playbackOptions -min 1 -max ";
	cmd = cmd + numFrames;
	dgMod.commandToExecute(cmd);

	cmd = "playbackOptions - ast 1 - aet ";
	cmd = cmd + numFrames;
	dgMod.commandToExecute(cmd);

	numFrames -= 1;

	// 
	for (unsigned int j = 0; j < weights.size(); j++) {

		for (unsigned int i = 0; i < weights.at(j).size(); i++) {
			cmd = "currentTime ";
			cmd = cmd + (j + 1);
			dgMod.commandToExecute(cmd);

			cmd = "setAttr shapesBS.weight[";
			cmd = cmd + i;
			cmd = cmd + "] ";
			cmd = cmd + weights.at(j).at(i);
			dgMod.commandToExecute(cmd);

			cmd = "setKeyframe { \"shapesBS.w[";
			cmd = cmd + i;
			cmd = cmd + "]\" }";
			dgMod.commandToExecute(cmd);
		}
	}
	return redoIt();
}

MStatus VfxCmd::undoIt() {
	return dgMod.undoIt();
}

MStatus VfxCmd::redoIt() {
	return dgMod.doIt();
}

bool VfxCmd::isUndoable() const {
	return true;
}

void *VfxCmd::creator() {
	return new VfxCmd;
}
