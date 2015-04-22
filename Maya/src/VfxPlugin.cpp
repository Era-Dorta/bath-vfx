#include "VfxCmd.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus stat;
	MString errStr;
	MFnPlugin plugin(obj);

	stat = plugin.registerCommand("VfxDo", VfxCmd::creator);
	if (!stat) {
		errStr = "registerCommand failed";
		goto error;
	}

	return stat;

	error:

	stat.perror(errStr);
	return stat;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus stat;
	MString errStr;
	MFnPlugin plugin(obj);

	stat = plugin.deregisterCommand("VfxDo");
	if (!stat) {
		errStr = "deregisterCommand failed";
		goto error;
	}

	return stat;

	error:

	stat.perror(errStr);
	return stat;
}
