#ifndef VFXCMD_H
#define VFXCMD_H

#include <vector>

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDaGPath.h>
#include <maya/MFnSet.h>
#include <maya/MFnDagNode.h>

class VfxCmd : public MPxCommand
{
public:
	virtual MStatus	doIt(const MArgList&);
	virtual MStatus undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; }

	static void *creator() { return new VfxCmd; }
	static MSyntax newSyntax();

private:
	static std::vector<std::string> attrNames;
	std::vector<unsigned int> attrIndices;
private:
	MDGModifier dgMod;
};

#endif