#include "Player.h"
#include "Room.h"
#include "Level.h"

Player::Player(cVector3d loc, cToolCursor* cursor, cWorld* world)
{
	speed = 0.02;
	armDisplacement = cVector3d(-0.02, 0.0, 0.01);
	body = new cShapeBox(0.0025, 0.0025, 0.008);
	body->setLocalPos(loc);
	this->tool = cursor;
	world->addChild(body);
	this->insideWall = false;
}


void Player::translate(cVector3d dir, double delta_t)
{
	cVector3d curP = body->getLocalPos();
	body->setLocalPos(curP + cVector3d(dir.x()*speed*delta_t, dir.y()*speed*delta_t, dir.z()*speed*delta_t));
}

int Player::getRoom(Level* lvl)
{
	double x = tool->getLocalPos().x() + tool->getDeviceLocalPos().x();
	double y = tool->getLocalPos().y() + tool->getDeviceLocalPos().y();
	x -= Room::sideLengthX / 2.;
	y += Room::sideLengthY / 2.;

	int i = 0;
	x += Room::sideLengthX;
	while (x < 0)
	{
		x += Room::sideLengthX;
		i++;
	}

	int j = 0;
	y -= Room::sideLengthX;
	while (y > 0)
	{
		y -= Room::sideLengthX;
		j++;
	}


	return i * 4 + j;
}