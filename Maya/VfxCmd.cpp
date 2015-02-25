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

std::vector<std::string> VfxCmd::attrNames{ "right_mouth_corner_up", "right_mouth_corner_down",
"right_mouth_corner_left", "right_mouth_corner_right", "right_mouth_corner_fwd",
"right_mouth_corner_back", "upper_lip_right", "upper_lip_right_down", "upper_lip_right_up",
"upper_lip_mid_down", "upper_lip_mid_up", "upper_lip_left", "upper_lip_left_down", "upper_lip_left_up",
"left_mouth_corner_up", "left_mouth_corner_down", "left_mouth_corner_left", "left_mouth_corner_right",
"left_mouth_corner_fwd", "left_mouth_corner_back", "lower_lip_left", "lower_lip_left_down",
"lower_lip_left_up", "lower_lip_mid_down", "lower_lip_mid_up", "lower_lip_right",
"lower_lip_right_up", "lower_lip_right_down", "left_blink", "right_blink", "nostrils_flare",
"nostrils_narrow", "nose_stretch_downwards", "left_frown", "right_frown", "ch", "mbp",
"normalFV", "strongFV", "extreme_mouth_narrow", "upper_lip_roll_out", "upper_lip_roll_in",
"lower_lip_roll_out", "lower_lip_roll_in", "smile_left", "smile_right", "left_blow",
"right_blow", "left_suck", "right_suck", "upper_lip_fwd", "upper_lip_back", "lower_lip_fwd",
"lower_lip_back", "mouth_up", "mouth_down", "sneer_right", "right_cheek_squint", "sneer_left",
"left_cheek_squint", "left_brow_up", "left_brow_down", "left_brow_squeeze", "right_brow_up",
"right_brow_down", "right_brow_squeeze", "mid_brows_up", "mid_brows_down", "brows_squeeze",
"im_jaw_open", "im_jaw_close", "upper_lip_thick_mid", "upper_lip_thick_right",
"upper_lip_thick_left", "lower_lip_thick_mid", "lower_lip_thick_right", "lower_lip_thick_left",
"upper_lip_thin_mid", "upper_lip_thin_right", "upper_lip_thin_left", "lower_lip_thin_left",
"lower_lip_thin_mid", "lower_lip_thin_right", "corners_wide_left", "corners_wide_right",
"corners_narrow_right", "corners_narrow_left", "scrunch_left", "scrunch_right",
"left_lip_narrow", "right_lip_narrow", "left_lip_wide", "right_lip_wide", "oo", "oo_tight" };

typedef std::vector < unsigned int >::iterator vecUintIt;

MStatus VfxCmd::doIt(const MArgList &args)
{
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

	// Add blenshapes to the selection list
	stat = MGlobal::getSelectionListByName("pup:allBlends", selection);
	if (checkStat(stat, AT)){
		return MS::kFailure;
	}

	// Get the DagPath of the blendshapes
	MDagPath blendShapePath;
	stat = selection.getDagPath(0, blendShapePath);
	if (checkStat(stat, AT)){
		return MS::kFailure;
	}

	MFnDagNode blendShapeFn(blendShapePath);

	// Go through all the plugs and save the index of the ones in 
	// the attributes vector names
	unsigned int attrCount = blendShapeFn.attributeCount();
	for (unsigned int i = 0; i < attrCount; i++){
		MObject attr = blendShapeFn.attribute(i);
		MFnAttribute mfnAttr(attr);
		MPlug plug = blendShapeFn.findPlug(attr, true);

		if (!mfnAttr.isReadable() || plug.isNull())
			continue;

		MString propName = plug.partialName(0, 0, 0, 0, 0, 1);
		std::string propStr = propName.asChar();

		std::vector<std::string>::iterator attrNamesIt;
		attrNamesIt = std::find(attrNames.begin(), attrNames.end(), propStr);
		if (attrNamesIt != attrNames.end()){
			attrIndices.push_back(i);
		}
	}

	// Set the value of all the blendshapes to 0.5
	vecUintIt i = attrIndices.begin();
	for (; i != attrIndices.end(); ++i){
		MObject attr = blendShapeFn.attribute(*i);
		MPlug plug = blendShapeFn.findPlug(attr, true);
		dgMod.newPlugValueFloat(plug, 0.5);
	}

	return redoIt();
}

MStatus VfxCmd::undoIt()
{
	return dgMod.undoIt();
}

MStatus VfxCmd::redoIt()
{
	return dgMod.doIt();
}
