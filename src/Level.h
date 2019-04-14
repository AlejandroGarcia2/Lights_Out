#pragma once
#include "chai3d.h"
#include "Room.h"
#include "Player.h"
#include <vector>

using namespace chai3d;
using namespace std;


class Level
{
public:

	Level(cWorld*, Player*);
	void createMazeFromFile(cWorld*,std::string);
	void initAudio();
	void startAudio();
	double getInBoundsX(double);
	double getInBoundsY(double);

	cAudioBuffer* buffers[5];
	cAudioSource* sources[5];

	cAudioBuffer* effectBuffers[3];
	cAudioSource* effectSources[3];

	int sourceTarget;
	double sourceTargetValue;
	cPrecisionClock *playTime;

	// audio device to play sound
	cAudioDevice* audioDevice;

	

	Player* player;
	vector<Room*> rooms;
};

