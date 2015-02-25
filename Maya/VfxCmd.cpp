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
#include <assert.h>

MStatus VfxCmd::doIt(const MArgList &args)
{
	// This command takes the selected mesh node, and conects its output to
	// a new shape interpolation node, then creates a new mesh node whose 
	// input is the output of the interpolation node
	MStatus stat;
	MSelectionList selection;

	// Get the initial shading group
	MObject shadingGroupObj;

	// N.B. Ensure the selection is list empty beforehand since
	// getSelectionListByName() will append the matching objects
	selection.clear();

	MGlobal::getSelectionListByName("initialShadingGroup", selection);
	selection.getDependNode(0, shadingGroupObj);
	MFnSet shadingGroupFn(shadingGroupObj);

	// Get a list of currently selected objects
	selection.clear();
	MGlobal::getActiveSelectionList(selection);
	MItSelectionList iter(selection);

	unsigned int count = 0;

	MDagPath sourceShapePath;
	MDagPath targetShapePath;

	// Iterate over all the mesh nodes
	iter.setFilter(MFn::kMesh);
	for (iter.reset(); !iter.isDone(); iter.next())
	{
		MDagPath geomShapePath;
		iter.getDagPath(geomShapePath);

		MDagPath geomTransformPath(geomShapePath);
		// Take the shape node out of the path
		geomTransformPath.pop();

		MFnDagNode geomShapeFn(geomShapePath);

		if (geomShapeFn.name().indexW("source") != -1){
			sourceShapePath = geomShapePath;
			count++;
		}
		else if (geomShapeFn.name().indexW("target") != -1){
			targetShapePath = geomShapePath;
			count++;
		}
		else if (count == 2){
			break;
		}
	}

	if (count < 2)
	{
		MGlobal::displayError("\nSelect source and target shape.");
		return MS::kFailure;
	}

	// TODO Move to constructor
	newNodeName = "vfxNode";

	duplicateMesh(shadingGroupFn, sourceShapePath, targetShapePath);

	//MString n( meltFn.name() );
	//MGlobal::displayInfo( "\nMelt node: " + name + " " + shapeFn.name() );

	MTime startTime = MAnimControl::minTime();
	MTime endTime = MAnimControl::maxTime();

	MString cmd;
	cmd = MString("setKeyframe -at interpolateValue -t ") + startTime.value() + " -v " + 0.0 + " " + newNodeName;
	dgMod.commandToExecute(cmd);

	cmd = MString("setKeyframe -at interpolateValue -t ") + endTime.value() + " -v " + 1.0 + " " + newNodeName;
	dgMod.commandToExecute(cmd);

	return redoIt();
}

MStatus VfxCmd::undoIt()
{
	MStatus status;

	if (doMeshUndo)
	{
		MGlobal::deleteNode(newMesh);
		doMeshUndo = false;
	}

	return dgMod.undoIt();
}

MStatus VfxCmd::redoIt()
{
	return dgMod.doIt();
}

void VfxCmd::duplicateMesh(MFnSet &shadingGroupFn, MDagPath &sourceShapePath,
	MDagPath &targetShapePath){

	MFnDagNode sourceShapeFn(sourceShapePath);

	// Duplicate the mesh
	// duplicate(bool instance = false, bool instaceLeaf = false)
	// Set to true to create instances, instanceLeaf controls whether the instance is
	// only at the leaf, returns a reference to the new Node
	MObject intrpMeshTransform = sourceShapeFn.duplicate(false, false);

	// Save the dag path of the newly created mesh
	newMesh = intrpMeshTransform;
	doMeshUndo = true;

	// Get MFnDagNode of the Shape Node of the interpolated mesh
	MFnDagNode intrpMeshShapeFn(intrpMeshTransform);
	intrpMeshShapeFn.setObject(intrpMeshShapeFn.child(0));

	// Assign the new mesh to the shading group so it will be rendered
	shadingGroupFn.addMember(intrpMeshShapeFn.object());

	// Create VfxNode node
	MObject intrpltNode = dgMod.createNode(VfxNode::id);
	assert(!intrpltNode.isNull());

	// Give the node a custom name
	dgMod.renameNode(intrpltNode, newNodeName);

	// Get plugs for the interpolator node
	MFnDependencyNode intrpltNodeFn(intrpltNode);
	MPlug sourceSurfacePlug = intrpltNodeFn.findPlug("sourceSurface");
	MPlug targetSurfacePlug = intrpltNodeFn.findPlug("targetSurface");
	MPlug outputSurfacePlug = intrpltNodeFn.findPlug("outputSurface");

	// Get source and targe mesh plugs
	MFnDagNode targetShapeFn(targetShapePath);
	MPlug sourcePlug = sourceShapeFn.findPlug("worldMesh");
	MPlug targetPlug = targetShapeFn.findPlug("worldMesh");

	// Set the plug to the correct element in the array
	unsigned int instanceNum = sourceShapePath.instanceNumber();
	sourcePlug.selectAncestorLogicalIndex(instanceNum);

	instanceNum = targetShapePath.instanceNumber();
	targetPlug.selectAncestorLogicalIndex(instanceNum);

	// Get input plug for the interpolated mesh shape
	MPlug intrpMeshPlug = intrpMeshShapeFn.findPlug("inMesh");

	// Conect the inputs of the Interpolator Node
	dgMod.connect(sourcePlug, sourceSurfacePlug);
	dgMod.connect(targetPlug, targetSurfacePlug);

	// Conect the outputs of the Interpolator Node
	dgMod.connect(outputSurfacePlug, intrpMeshPlug);
}