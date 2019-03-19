#pragma once
#include "chai3d.h"
#include <vector>

using namespace chai3d;

class Wall
{
public:
	Wall(std::string);
	void initAudio(cAudioDevice*);

	cMesh* mesh;
	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;

	std::string source;
};

