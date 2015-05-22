#ifndef VFXCMD_H
#define VFXCMD_H

#include <vector>

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDagPath.h>
#include <maya/MFnSet.h>
#include <maya/MFnDagNode.h>
#include <maya/MTime.h>
#include <maya/MArgList.h>

class VfxCmd: public MPxCommand {

	enum Actions {
		SAVE, LOAD
	};

public:
	virtual MStatus doIt(const MArgList&args);
	virtual MStatus undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const;
	static void *creator();
	static MSyntax newSyntax();
private:
	void loadWeights(int numWeights);
	void saveWeights();
	void setBlinkAt(int blinkFrame, float blinkVal);
	unsigned int readWeigths(int numWeights,
			std::vector<std::vector<float> >& weights);
	void readRot(unsigned int numFrames);

private:
	Actions action;
	int numWeights;
	std::vector<unsigned int> attrIndices;
	std::vector<std::vector<double>> invRot;
	static std::vector<unsigned int> blinkFrames;
	static std::vector<unsigned int> blinkTime;
	static std::vector<float> blinkWeight;
	const static MString names[];
	int file_ind;
	MDGModifier dgMod;
	MTime prevMinTime;
	MTime prevMaxTime;
	MTime prevStartTime;
	MTime prevEndTime;
};

#endif
