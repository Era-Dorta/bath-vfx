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

std::vector<unsigned int> VfxCmd::blinkStart = { 680, 756, 792 };
std::vector<unsigned int> VfxCmd::blinkClose = { 687, 765, 800 };
std::vector<unsigned int> VfxCmd::blinkOpen = { 743, 783, 818 };
std::vector<unsigned int> VfxCmd::blinkEnd = { 751, 792, 827 };
std::vector<float> VfxCmd::blinkWeight = { 0.4, 0.8, 0.8 };
std::vector<VfxCmd::Eye> VfxCmd::blinkEye = { BOTH, RIGHT, LEFT };

#ifdef _WIN32
#define LOAD_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_6.txt"
#define SAVE_WEIGHTS_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\weights_out"
#define ROTATION_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\invRotation.txt"
#define TRANSLATION_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\translation.txt"
#define LEFT_EYE_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\left_eye.txt"
#define RIGHT_EYE_PATH "C:\\Users\\Ieva\\Dropbox\\Semester2\\VFX\\Matlab\\Transformation\\data\\right_eye.txt"
#else
#define LOAD_WEIGHTS_PATH "/home/gdp24/workspaces/matlab/vfx/Data/Transformation/weights5.txt"
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
	eyeOrientationScale = 1;

	customFrameRange = false;
	startFrame = 650;
	endFrame = 850;

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

	readOrientationFile(numFrames);

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
	if (customFrameRange) {
		anim.setMinMaxTime(MTime((double) startFrame),
				MTime((double) endFrame));
		anim.setAnimationStartEndTime(MTime((double) startFrame),
				MTime((double) endFrame));
	} else {
		anim.setMinMaxTime(MTime(1.0), MTime((double) (numFrames)));
		anim.setAnimationStartEndTime(MTime(1.0), MTime((double) (numFrames)));
	}

	cmd = "select -r geoGroup";
	dgMod.commandToExecute(cmd);

	//Set numFrames to start at 0 for c++ indexing
	numFrames -= 1;
	for (unsigned int i = 0; i < weights.size(); i++) {
		if (customFrameRange && i < startFrame) {
			continue;
		}

		cmd = "currentTime ";
		cmd = cmd + (i + 1);
		dgMod.commandToExecute(cmd);

		// Set head movement
		cmd = "xform -m 1 0 0 0 0 1 0 0 0 0 1 0";
		// Translation
		for (unsigned int j = 0; j < 3; j++) {
			cmd += " ";
			cmd += invTransform.at(i)(3, j);
		}
		// Homogeneus scale value in the matrix
		cmd += " 1";
		dgMod.commandToExecute(cmd);

		cmd = "xform -r -m ";
		// Rotation part
		for (unsigned int row = 0; row < 3; row++) {
			cmd += " ";
			cmd += invTransform.at(i)(row, 0);
			cmd += " ";
			cmd += invTransform.at(i)(row, 1);
			cmd += " ";
			cmd += invTransform.at(i)(row, 2);
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
		cmd += eyeOrientation[i].x;
		cmd += " ";
		cmd += eyeOrientation[i].y;
		cmd += " 0";
		dgMod.commandToExecute(cmd);

		cmd = "setKeyframe con_lookAt_c";
		dgMod.commandToExecute(cmd);

		// Set keyframe for head movement
		cmd = "setKeyframe";
		dgMod.commandToExecute(cmd);
	}

	// Set keyframes for blinking
	for (unsigned int i = 0; i < blinkStart.size(); i++) {
		setBlinkAt(i, blinkStart.at(i), 0.0);
		setBlinkAt(i, blinkClose.at(i), blinkWeight.at(i));
		setBlinkAt(i, blinkOpen.at(i), blinkWeight.at(i));
		setBlinkAt(i, blinkEnd.at(i), 0.0);
	}

	// Reset selection to none
	cmd = "select -cl";
	dgMod.commandToExecute(cmd);

	// Reset current time to 0
	cmd = "currentTime ";
	if (customFrameRange) {
		cmd += startFrame - 1;
	} else {
		cmd += "0";
	}
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

void VfxCmd::setBlinkLeft(float blinkVal) {
	MString cmd = "setAttr \"shapesBS.eye_blink2_l\"";
	cmd = cmd + blinkVal;
	dgMod.commandToExecute(cmd);
	cmd = "setKeyframe \"shapesBS.w[13]\"";
	dgMod.commandToExecute(cmd);
}

void VfxCmd::setBlinkRight(float blinkVal) {
	MString cmd = "setAttr \"shapesBS.eye_blink2_r\"";
	cmd = cmd + blinkVal;
	dgMod.commandToExecute(cmd);
	cmd = "setKeyframe \"shapesBS.w[14]\"";
	dgMod.commandToExecute(cmd);
}

void VfxCmd::setBlinkAt(unsigned int blinkInd, int frameNum, float blinkVal) {
	MString cmd;
	cmd = "currentTime ";
	cmd = cmd + frameNum;
	dgMod.commandToExecute(cmd);

	switch (blinkEye.at(blinkInd)) {
	case LEFT: {
		setBlinkLeft(blinkVal);
		break;
	}
	case RIGHT: {
		setBlinkRight(blinkVal);
		break;
	}
	case BOTH: {
		setBlinkLeft(blinkVal);
		setBlinkRight(blinkVal);
		break;
	}
	}
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
			if (customFrameRange && numFrames == endFrame) {
				break;
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

	invTransform.resize(numFrames);
	std::fstream rotationFile( ROTATION_PATH, std::ios_base::in);
	std::fstream translationFile( TRANSLATION_PATH, std::ios_base::in);
	if (rotationFile.is_open()) {
		if (translationFile.is_open()) {
			for (unsigned int i = 0; i < numFrames; i++) {

				// Read them transpose, because Maya is vector row and Matlab
				// is vector column
				for (unsigned int row = 0; row < 3; row++) {
					for (unsigned int col = 0; col < 3; col++) {
						std::getline(rotationFile, line);
						invTransform.at(i)(row, col) = std::stof(line);
					}
				}

				for (unsigned int j = 0; j < 3; j++) {
					std::getline(translationFile, line);
					// Negative of the translation since we are replicating the movement
					// not correcting it
					invTransform.at(i)(3, j) = -std::stof(line)
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

void VfxCmd::readOrientationFile(unsigned int numFrames) {
	std::string line;

	eyeOrientation = MFloatVectorArray(numFrames);
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

				float alpha = 0.5, beta = 0.5, outTh = 1.0;
				float m = -0.5f / 3.0f, c = -m * 4.0f;
				// Check for fail tracking in only one eye, if so just force the
				// fail one to follow the good one
				if (leftCurrentPos.x > outTh || leftCurrentPos.x < -outTh) {
					if (rightCurrentPos.x <= outTh
							&& rightCurrentPos.x >= -outTh) {
						alpha = c + m * fabs(leftCurrentPos.x);
						beta = 1.0f - alpha;
					}
				} else {
					if (rightCurrentPos.x > outTh
							|| rightCurrentPos.x < -outTh) {
						beta = c + m * fabs(rightCurrentPos.x);
						alpha = 1.0f - beta;
					}
				}

				if (leftCurrentPos.y > outTh || leftCurrentPos.y < -outTh) {
					if (rightCurrentPos.y <= outTh
							&& rightCurrentPos.y >= -outTh) {
						alpha = c + m * fabs(leftCurrentPos.y);
						beta = 1.0f - alpha;
					}
				} else {
					if (rightCurrentPos.y > outTh
							|| rightCurrentPos.y < -outTh) {
						beta = c + m * fabs(rightCurrentPos.y);
						alpha = 1.0f - beta;
					}
				}

				// It looks weird to have both eyes moving differently, do the
				// mean and make them move together
				eyeOrientation[i] = (alpha * leftCurrentPos
						+ beta * rightCurrentPos) * eyeOrientationScale;
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
