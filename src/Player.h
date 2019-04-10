#pragma once
#include "chai3d.h"
#include "Room.h"

using namespace chai3d;

class Level;

class Player 
{
public:

	Player(cVector3d, cToolCursor*, cWorld*);
	void translate(cVector3d, double);
	cVector3d translateThroughDevice(double delta_t);
	//The room in which we are
	int getRoom(Level*);
	void toggleEarOnWall();
	void initAudio(std::string source, cAudioDevice* audioDevice);


	//Pos of the actual physical haptic tip in the world
	cVector3d getPhysicalPosition();
	//Pos of the proxy in the world
	cVector3d getPosition();



	cToolCursor* tool;
	//arm displacement
	double speed; // m/s
	bool earUpAgainstWall;
	cAudioBuffer* wallSoundBuffer;
	cAudioSource* wallSoundSource;
};