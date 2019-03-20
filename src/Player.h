#pragma once
#include "chai3d.h"
#include "Room.h"

using namespace chai3d;

class Player 
{
public:

	Player(cVector3d, cShapeSphere*, cWorld*);
	void translate(cVector3d, double);
	cShapeBox* body;
	cShapeSphere* head;
	cShapeSphere* cursor;
	//arm displacement
	cVector3d armDisplacement;

	//The room in which we are
	Room* room;

	double speed; // m/s
	bool insideWall;
};