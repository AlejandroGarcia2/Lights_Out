#pragma once
#include "chai3d.h"
#include <vector>

using namespace chai3d;

class Wall
{
public:
	Wall(cVector3d pos, cMatrix3d rot, std::string tex);
	void initAudio(std::string source, cAudioDevice* audioDevice, cVector3d audioPos);
	void setAudioPos(cVector3d pos) { audioSource->setSourcePos(pos); };

	cMesh* mesh;
	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;

	static double scaleFactor, sideLength;
	std::string source;
	cVector3d audioPos;
};

