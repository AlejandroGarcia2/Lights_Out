#pragma once
#include "chai3d.h"
#include <vector>

using namespace chai3d;

class Wall
{
public:
	Wall(std::string, cVector3d);
	void initAudio(cAudioDevice*);
	void setAudioPos(cVector3d pos) { audioSource->setSourcePos(pos); };

	cMesh* mesh;
	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;

	std::string source;
	cVector3d audioPos;
};

