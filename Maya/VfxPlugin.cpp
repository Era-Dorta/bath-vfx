#include "VfxNode.h"
#include "VfxCmd.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj)
{
	MStatus stat;
	MString errStr;
	MFnPlugin plugin(obj);

	stat = plugin.registerCommand("vfxDo", VfxCmd::creator);
	if (!stat)
	{
		errStr = "registerCommand failed";
		goto error;
	}

	stat = plugin.registerNode("intrpl", VfxNode::id,
		VfxNode::creator, VfxNode::initialize);
	if (!stat)
	{
		errStr = "registerNode failed";
		goto error;
	}

	return stat;

error:

	stat.perror(errStr);
	return stat;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus stat;
	MString errStr;
	MFnPlugin plugin(obj);

	stat = plugin.deregisterCommand("vfxDo");
	if (!stat)
	{
		errStr = "deregisterCommand failed";
		goto error;
	}

	stat = plugin.deregisterNode(VfxNode::id);
	if (!stat)
	{
		errStr = "deregisterNode failed";
		goto error;
	}

	return stat;

error:

	stat.perror(errStr);
	return stat;
}
