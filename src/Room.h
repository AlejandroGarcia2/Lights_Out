#pragma once
#include "chai3d.h"
#include <vector>
#include "Wall.h"

using namespace chai3d;

#define front 0
#define left 1
#define right 2
#define back 3
#define top 4
#define bot 5

class Player; 

class Room
{
public:
	Room(cWorld*, cAudioDevice*, cVector3d, bool[6], int);
	void initAudio(std::string source, cAudioDevice* audioDevice);
	void setAudioPos(cVector3d pos) { audioSource->setSourcePos(pos); };
	
	Wall* walls[6];

	static double sideLengthX, sideLengthY, sideLengthZ, scaleFactor;

	int muffleLevel;

	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;

	std::string source;
	cVector3d position;

	bool activated[6];
};

