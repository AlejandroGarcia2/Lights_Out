#pragma once
#include "chai3d.h"
#include "Room.h"
#include "AudioFile.h"

using namespace chai3d;

class Level;

class Player 
{
public:

	Player(cVector3d, cToolCursor*, cWorld*);
	void translate(cVector3d, double);
	cVector3d translateThroughDevice(double delta_t);
	cVector3d handleMovement(cVector3d&, double, Level*, double);
	//The room in which we are
	int getRoom(Level*, cVector3d);
	void toggleEarOnWall();
	void initAudio(std::string file, cAudioDevice* audioDevice);
	void handleFootsteps(cAudioDevice*);

	//Pos of the actual physical haptic tip in the world
	cVector3d getPhysicalPosition();
	//Pos of the player
	cVector3d getPosition();
	//Final rendered position of the tool
	cVector3d getProxyPosition();


	//Index of the room we are in
	int room;
	cToolCursor* tool;
	//arm displacement
	double speed; // m/s
	bool micUpAgainstWall;
	cAudioBuffer* wallSoundBuffer;
	cAudioSource* wallSoundSource;

	cShapeBox* body;
	cShapeLine* arm;
	cShapeBox* micBase;
	cShapeLine* leftArm;

	bool validMove;
	bool footstepsPlaying, startPlayingFootsteps, stopPlayingFootsteps;
	cAudioBuffer* footstepsSoundBuffer;
	cAudioSource* footstepsSoundSource;
	cAudioBuffer* footstepsThumpSoundBuffer;

	cPrecisionClock* moveClock;
	cVector3d prevPos, targetPos;

	AudioFile<double> crashAudioFile;
	cVector3d prevForce;
};