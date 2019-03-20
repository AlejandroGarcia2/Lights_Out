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
	Room(cWorld*, cAudioDevice*, cVector3d);
	void initAudio(std::string source, cAudioDevice* audioDevice);
	void setAudioPos(cVector3d pos) { audioSource->setSourcePos(pos); };

	// Compute the force to constrain the cursor inside the room (the force due to the 6 planes)
	cVector3d computeForceDueToRoom(cVector3d, Player*);
	void setEar(Wall* wall, double penetration);

	Wall* walls[6];
	Room* rooms[4];

	cAudioBuffer* audioBuffer;
	cAudioSource* audioSource;

	std::string source;
	cVector3d position;
};

