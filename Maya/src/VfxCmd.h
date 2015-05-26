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
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatMatrix.h>

class VfxCmd: public MPxCommand {

	enum Actions {
		SAVE, LOAD
	};

	enum Eye {
		LEFT, RIGHT, BOTH
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
	void setBlinkAt(unsigned int blinkInd, int blinkFrame, float blinkVal);
	unsigned int readWeights(int numWeights,
			std::vector<std::vector<float> >& weights);
	void readTransMatrixFile(unsigned int numFrames);
	void readOrientationFile(unsigned int numFrames);
	void setBlinkLeft(float blinkVal);
	void setBlinkRight(float blinkVal);

private:
	Actions action;
	int numWeights;
	std::vector<unsigned int> attrIndices;
	std::vector<MFloatMatrix> invTransform;
	float translationScale;
	float eyeOrientationScale;
	MFloatVectorArray eyeOrientation;
	static std::vector<unsigned int> blinkStart;
	static std::vector<unsigned int> blinkClose;
	static std::vector<unsigned int> blinkOpen;
	static std::vector<unsigned int> blinkEnd;
	static std::vector<float> blinkWeight;
	static std::vector<Eye> blinkEye;
	const static MString names[];
	int file_ind;
	MDGModifier dgMod;
	MTime prevMinTime;
	MTime prevMaxTime;
	MTime prevStartTime;
	MTime prevEndTime;
	unsigned int startFrame;
	unsigned int endFrame;
	bool customFrameRange;
};

#endif
