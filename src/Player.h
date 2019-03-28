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
	cShapeBox* body;
	cShapeSphere* head;
	cToolCursor* tool;
	//arm displacement
	cVector3d armDisplacement;
	cShapeLine* arm;

	//The room in which we are
	int getRoom(Level*);

	double speed; // m/s
	bool insideWall;
};