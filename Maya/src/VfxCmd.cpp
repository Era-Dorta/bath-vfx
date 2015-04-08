#include "VfxCmd.h"
#include "VfxNode.h"
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

#include <string>
#include "ErrorCheck.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT " " __FILE__ ": " TOSTRING(__LINE__) 

typedef std::vector<unsigned int>::iterator vecUintIt;

MStatus VfxCmd::doIt(const MArgList &args) {
	// This command takes the selected mesh node, and conects its output to
	// a new shape interpolation node, then creates a new mesh node whose 
	// input is the output of the interpolation node
	MStatus stat;
	MSelectionList selection;

	// Get the initial shading group
	MObject blendShapeObj;

	// N.B. Ensure the selection is list empty beforehand since
	// getSelectionListByName() will append the matching objects
	selection.clear();

	// TODO Change pup:allBlends for name given by parameter
	// Add blenshapes to the selection list
	/*stat = MGlobal::getSelectionListByName("shapesBS", selection);
	if (checkStat(stat, AT)) {
		return MS::kFailure;
	}

	// Get the DagPath of the blendshapes
	MDagPath blendShapePath;
	stat = selection.getDagPath(0, blendShapePath);
	if (checkStat(stat, AT)) {
		return MS::kFailure;
	}

	MFnDagNode blendShapeFn(blendShapePath);*/

	// Get all user defined attributes
	MString cmd("getAttr -s shapesBS.weight");
	int numWeights;
	MGlobal::executeCommand(cmd, numWeights);
	numWeights -= 1;

	// Go through all the plugs and save the index of valid ones
	for (unsigned int i = 0; i < numWeights; i++) {


		cmd = "setAttr shapesBS.weight[";
		cmd = cmd + i;
		cmd = cmd +"] ";
		cmd = cmd + 0.01;
		MGlobal::executeCommand(cmd);
		
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