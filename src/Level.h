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
	bool isInBoundsX(double);
	bool isInBoundsY(double);
	bool collides(const cVector3d&);

	cAudioBuffer* buffers[5];
	cAudioSource* sources[5];

	vector<cAudioBuffer*> effectBuffers;
	cAudioSource* effectSource;

	cAudioBuffer* winBuffer;
	cAudioBuffer* loseBuffer;
	cAudioBuffer* lightsOutBuffer;
	cAudioSource* winLoseSource;

	vector<int> deadEndRooms;
	int endRoom;

	cSpotLight* spotLight;

	int sourceTarget;
	double sourceTargetValue;
	cPrecisionClock *playTime;

	// audio device to play sound
	cAudioDevice* audioDevice;

	

	Player* player;
	vector<Room*> rooms;
};

