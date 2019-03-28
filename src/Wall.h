#pragma once
#include "chai3d.h"
#include <vector>

using namespace chai3d;

class Wall
{
public:
	Wall(cVector3d pos, cMatrix3d rot, std::string tex, double w, double h);
	void initAudio(std::string source, cAudioDevice* audioDevice, cVector3d audioPos);
	void setAudioPos(cVector3d pos) { audioSource->setSourcePos(pos); };
	void setVisible(bool);

	cMesh* mesh;
	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;
	bool active;

	std::string source;
	cVector3d audioPos;
};

