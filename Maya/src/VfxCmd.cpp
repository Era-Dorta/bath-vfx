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
#include <cmath>

#define TO_DEG 180.0 / M_PI

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

std::vector<unsigned int> VfxCmd::blinkFrames { 48, 322, 474, 575, 588 };
std::vector<unsigned int> VfxCmd::blinkTime = { 7, 10, 8, 8, 6 };
std::vector<float> VfxCmd::blinkWeight = { 0.9, 1, 0.5, 0.4, 0.5 };

#ifdef _WIN32
#define LOAD_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_6.txt"
#define SAVE_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_out"
#define ROTATION_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\invRotation.txt"
#define TRANSLATION_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\translation.txt"
#define LEFT_EYE_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\left_eye.txt"
#define RIGHT_EYE_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\right_eye.txt"
#else
#define LOAD_WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights2.txt"
#define SAVE_WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights_out"
#define ROTATION_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/invRotation.txt"
#define TRANSLATION_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/translation.txt"
#define LEFT_EYE_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/left_eye.txt"
#define RIGHT_EYE_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/right_eye.txt"
#endif

MStatus VfxCmd::doIt(const MArgList &args) {

	MStatus stat;

	action = SAVE;
	file_ind = 0;
	translationScale = 0.1;
	eyeRotScale = 2;

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

	std::vector<std::vector<float> > weights;
	// Read all the weights from the weights file
	unsigned int numFrames = readWeights(numWeights, weights);

	// Read the rotation matrix from the rotation file
	readTransMatrixFile(numFrames);

	readEyeRotationFile(numFrames);

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

	cmd = "select -r geoGroup";
	dgMod.commandToExecute(cmd);

	//Set numFrames to start at 0 for c++ indexing
	numFrames -= 1;
	for (unsigned int i = 0; i < weights.size(); i++) {
		cmd = "currentTime ";
		cmd = cmd + (i + 1);
		dgMod.commandToExecute(cmd);

		// Set head movement
		cmd = "xform -m 1 0 0 0 0 1 0 0 0 0 1 0";
		// Translation
		for (unsigned int j = 9; j < 12; j++) {
			cmd += " ";
			cmd += invTransform.at(i).at(j);
		}
		// Homogeneus scale value in the matrix
		cmd += " 1";
		dgMod.commandToExecute(cmd);

		cmd = "xform -r -m ";
		// Rotation part
		for (unsigned int j = 0; j < 3; j++) {
			cmd += " ";
			cmd += invTransform.at(i).at(j * 3);
			cmd += " ";
			cmd += invTransform.at(i).at(j * 3 + 1);
			cmd += " ";
			cmd += invTransform.at(i).at(j * 3 + 2);
			cmd += " 0";
		}
		cmd += " 0 0 0 1";
		dgMod.commandToExecute(cmd);

		for (unsigned int j = 0; j < weights.at(i).size(); j++) {
			if (j != 13 && j != 14) {

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
		}
		// Set the teeth movement for this frame, interpolates between the two
		// blend shapes that can open the mouth
		cmd = "setAttr con_jaw_c.translateY ";
		cmd += -3.2 * (weights.at(i).at(58) + weights.at(i).at(55)) + 1;
		dgMod.commandToExecute(cmd);
		cmd = "setKeyframe  \"con_jaw_c.translateY\"";
		dgMod.commandToExecute(cmd);

		// Force a subtle smile on Emily
		cmd = "setAttr shapesBS.mouth_lipCornerPull_l 0.35";
		dgMod.commandToExecute(cmd);
		cmd = "setKeyframe shapesBS.mouth_lipCornerPull_l";
		dgMod.commandToExecute(cmd);

		cmd = "setAttr shapesBS.mouth_lipCornerPull_r 0.35";
		dgMod.commandToExecute(cmd);
		cmd = "setKeyframe shapesBS.mouth_lipCornerPull_r";
		dgMod.commandToExecute(cmd);

		cmd = "setAttr con_lookAt_c.t ";
		cmd += leftEyeRotation[i].x;
		cmd += " ";
		cmd += leftEyeRotation[i].y;
		cmd += " 0";
		dgMod.commandToExecute(cmd);

		cmd = "setKeyframe con_lookAt_c";
		dgMod.commandToExecute(cmd);

		// Set keyframe for head movement
		cmd = "setKeyframe";
		dgMod.commandToExecute(cmd);
	}

	// Set keyframes for blinking
	for (unsigned int i = 0; i < blinkFrames.size(); i++) {
		setBlinkAt(blinkFrames.at(i) - blinkTime.at(i), 0.0);
		setBlinkAt(blinkFrames.at(i), blinkWeight.at(i));
		setBlinkAt(blinkFrames.at(i) + blinkTime.at(i), 0.0);
	}

	// Reset selection to none
	cmd = "select -cl";
	dgMod.commandToExecute(cmd);

	// Reset current time to 0
	cmd = "currentTime 0";
	dgMod.commandToExecute(cmd);
}

void VfxCmd::saveWeights() {
	MStatus stat;
	std::string path = SAVE_WEIGHTS_PATH;
	path = path + std::to_string(file_ind) + ".txt";

	std::ifstream infile(path);
	bool extra_endl = false;
	if (infile.good()) {
		extra_endl = true;
	}
	infile.close();

	std::fstream myfile(path, std::ios_base::app | std::fstream::out);
	myfile.precision(10);
	MDoubleArray weights;

	MString cmd("getAttr shapesBS.weight");
	stat = MGlobal::executeCommand(cmd, weights);

	MGlobal::displayInfo(MString("Save path: ") + path.c_str());

	if (extra_endl) {
		myfile << endl;
	}

	for (unsigned int i = 0; i < weights.length() - 1; i++) {
		myfile << weights[i] << endl;
	}
	myfile << weights[weights.length() - 1];

	myfile.close();
}

void VfxCmd::setBlinkAt(int frameNum, float blinkVal) {
	MString cmd;
	cmd = "currentTime ";
	cmd = cmd + frameNum;
	dgMod.commandToExecute(cmd);

	cmd = "setAttr \"shapesBS.eye_blink2_l\"";
	cmd = cmd + blinkVal;
	dgMod.commandToExecute(cmd);

	cmd = "setAttr \"shapesBS.eye_blink2_r\"";
	cmd = cmd + blinkVal;
	dgMod.commandToExecute(cmd);

	cmd = "setKeyframe \"shapesBS.w[13]\"";
	dgMod.commandToExecute(cmd);

	cmd = "setKeyframe \"shapesBS.w[14]\"";
	dgMod.commandToExecute(cmd);
}

unsigned int VfxCmd::readWeights(int numWeights,
		std::vector<std::vector<float> >& weights) {
	unsigned int numFrames = 0;
	std::string line;
	unsigned int weightNum = 0;
	std::vector<float> currentWeights(numWeights, 0.0);
	std::fstream weightsFile( LOAD_WEIGHTS_PATH, std::ios_base::in);
	if (weightsFile.is_open()) {
		while (std::getline(weightsFile, line)) {
			// Save current weight
			currentWeights.at(weightNum) = std::stof(line);
			weightNum++;
			if (weightNum == (unsigned int) (((numWeights)))) {
				// Save weights for current frame in weights vector
				weights.push_back(currentWeights);
				std::fill(currentWeights.begin(), currentWeights.end(), 0);
				weightNum = 0;
				numFrames++;
			}
		}
		weightsFile.close();
	} else {
		MGlobal::displayWarning(
				"VfxCMD: could not read file " LOAD_WEIGHTS_PATH);
	}
	line = "VfxCMD: Read " + std::to_string(numFrames) + " frames";
	MGlobal::displayInfo(line.c_str());
	return numFrames;
}

void VfxCmd::readTransMatrixFile(unsigned int numFrames) {
	std::string line;
	std::vector<double> emptymatrix(12, 0);
	emptymatrix.at(0) = 1;
	emptymatrix.at(4) = 1;
	emptymatrix.at(8) = 1;

	invTransform.resize(numFrames, emptymatrix);
	std::fstream rotationFile( ROTATION_PATH, std::ios_base::in);
	std::fstream translationFile( TRANSLATION_PATH, std::ios_base::in);
	if (rotationFile.is_open()) {
		if (translationFile.is_open()) {
			for (unsigned int i = 0; i < numFrames; i++) {
				for (unsigned int j = 0; j < 9; j++) {
					std::getline(rotationFile, line);
					invTransform.at(i).at(j) = std::stod(line);
				}
				for (unsigned int j = 9; j < 12; j++) {
					std::getline(translationFile, line);
					// Negative of the translation since we are replicating the movement
					// not correcting it
					invTransform.at(i).at(j) = -std::stod(line)
							* translationScale;
				}
			}
			translationFile.close();
			rotationFile.close();
		} else {
			MGlobal::displayWarning(
					"VfxCMD: could not read file " TRANSLATION_PATH);
		}
	} else {
		MGlobal::displayWarning("VfxCMD: could not read file " ROTATION_PATH);
	}
}

void VfxCmd::readEyeRotationFile(unsigned int numFrames) {
	std::string line;

	leftEyeRotation = MFloatVectorArray(numFrames);
	rightEyeRotation = MFloatVectorArray(numFrames);
	std::fstream leftEyeFile( LEFT_EYE_PATH, std::ios_base::in);
	std::fstream rightEyeFile( RIGHT_EYE_PATH, std::ios_base::in);
	MFloatVector leftOrigin, rightOrigin, leftCurrentPos, rightCurrentPos;
	if (leftEyeFile.is_open()) {
		if (rightEyeFile.is_open()) {
			// Read first frame position
			for (unsigned int j = 0; j < 3; j++) {
				std::getline(leftEyeFile, line);
				leftOrigin[j] = std::stof(line);
				std::getline(rightEyeFile, line);
				rightOrigin[j] = std::stof(line);
			}

			// Read the rest
			for (unsigned int i = 1; i < numFrames; i++) {
				// Read eye position
				for (unsigned int j = 0; j < 3; j++) {
					std::getline(leftEyeFile, line);
					leftCurrentPos[j] = std::stof(line);
					std::getline(rightEyeFile, line);
					rightCurrentPos[j] = std::stof(line);
				}

				// Sin of the angle is current - origin
				leftCurrentPos = leftCurrentPos - leftOrigin;
				rightCurrentPos = rightCurrentPos - rightOrigin;

				leftEyeRotation[i] = leftCurrentPos * eyeRotScale;

				leftEyeRotation[i] = rightCurrentPos * eyeRotScale;

				// It looks weird to have both eyes moving differently, do the
				// mean and make them move together
				leftEyeRotation[i] = (leftEyeRotation[i] + rightEyeRotation[i])
						* 0.5;
				rightEyeRotation[i] = leftEyeRotation[i];
			}
			rightEyeFile.close();
			leftEyeFile.close();
		} else {
			MGlobal::displayWarning(
					"VfxCMD: could not read file " TRANSLATION_PATH);
		}
	} else {
		MGlobal::displayWarning("VfxCMD: could not read file " ROTATION_PATH);
	}
}
