#pragma once
#include "chai3d.h"
#include "Wall.h"
#include "Player.h"
#include <vector>

using namespace chai3d;

#define front 0
#define bot 1
#define left 2
#define right 3
#define top 4
#define back 5


class Level
{
public:

	
	Level(cWorld*, Player*);

	// Compute the force to constrain the cursor inside the room (the force due to the 5 planes)
	cVector3d computeForceDueToRoom(cVector3d);
	void setEar(Wall* wall);

	Wall* walls[6];

	double scaleFactor, offset, sideLength;
	Player* player;
	cWorld* world;

	// audio device to play sound
	cAudioDevice* audioDevice;

	// audio buffer to store a sound file
	cAudioBuffer* audioBuffer;

	// audio source which plays the audio buffer 
	cAudioSource* audioSource;
};

