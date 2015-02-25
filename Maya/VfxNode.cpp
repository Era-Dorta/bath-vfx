#include "VfxNode.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MPointArray.h>
#include <maya/MAngle.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <assert.h>
#include <float.h>

const MTypeId VfxNode::id(0x00335);

MObject VfxNode::sourceSurface;
MObject VfxNode::targetSurface;
MObject VfxNode::interpolateValue;
MObject VfxNode::outputSurface;

MStatus VfxNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	// If asked for the outputSurface then compute it
	if (plug == outputSurface)
	{
		// Get handlers for the two meshes
		MDataHandle sourceSurfaceHnd = data.inputValue(sourceSurface);
		MDataHandle targetSurfaceHnd = data.inputValue(targetSurface);
		MDataHandle interpolateValueHnd = data.inputValue(interpolateValue);
		MDataHandle outputSurfaceHnd = data.outputValue(outputSurface);

		// Copy input into output
		outputSurfaceHnd.copy(sourceSurfaceHnd);

		// Get interpolation value
		double interpolateVal = interpolateValueHnd.asDouble();

		// Dummy interpolation, just translate each vertex 1 on x
		MPoint sourcePoint, targetPoint, outputPoint;

		MItGeometry outputIt(outputSurfaceHnd, false);
		MItGeometry targetIt(targetSurfaceHnd, false);
		MItGeometry sourceIt(sourceSurfaceHnd, false);

		for (; !outputIt.isDone(); outputIt.next())
		{
			sourcePoint = sourceIt.position();
			targetPoint = targetIt.position();

			outputIt.setPosition((1 - interpolateVal) * sourcePoint +
				interpolateVal * targetPoint);

			targetIt.next();
			sourceIt.next();
		}

		// Tell de DG that we have updated the outputSurface
		data.setClean(plug);
	}
	else
		stat = MS::kUnknownParameter;

	return stat;
}

void *VfxNode::creator()
{
	return new VfxNode();
}

MStatus VfxNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;

	sourceSurface = tAttr.create("sourceSurface", "ss", MFnData::kMesh);
	tAttr.setHidden(true);

	targetSurface = tAttr.create("targetSurface", "ts", MFnData::kMesh);
	tAttr.setHidden(true);

	interpolateValue = nAttr.create("interpolateValue", "iv", MFnNumericData::kDouble, 0.0);
	nAttr.setKeyable(true);

	outputSurface = tAttr.create("outputSurface", "os", MFnData::kMesh);
	tAttr.setStorable(false);
	tAttr.setHidden(true);

	addAttribute(sourceSurface);
	addAttribute(targetSurface);
	addAttribute(interpolateValue);
	addAttribute(outputSurface);

	attributeAffects(sourceSurface, outputSurface);
	attributeAffects(targetSurface, outputSurface);
	attributeAffects(interpolateValue, outputSurface);

	return MS::kSuccess;
}