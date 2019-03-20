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

	// audio device to play sound
	cAudioDevice* audioDevice;

	Player* player;
	vector<Room*> rooms;
};

