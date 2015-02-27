#ifndef VFXCMD_H
#define VFXCMD_H

#include <vector>

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDagPath.h>
#include <maya/MFnSet.h>
#include <maya/MFnDagNode.h>

class VfxCmd: public MPxCommand {
public:
	virtual MStatus doIt(const MArgList&);
	virtual MStatus undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const;
	static void *creator();
	static MSyntax newSyntax();
private:
	std::vector<unsigned int> attrIndices;
private:
	MDGModifier dgMod;
};

#endif
