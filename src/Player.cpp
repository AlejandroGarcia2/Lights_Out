#include "Player.h"

Player::Player(cVector3d loc, cShapeSphere* cursor, cWorld* world)
{
	speed = 0.15;
	armDisplacement = cVector3d(-0.02, 0.0, 0.01);
	body = new cShapeBox(0.01, 0.01, 0.03);
	body->setLocalPos(loc);
	this->cursor = cursor;
	//world->addChild(body);
	this->insideWall = false;
}


void Player::translate(cVector3d dir, double delta_t)
{
	cVector3d curP = body->getLocalPos();
	body->setLocalPos(curP + cVector3d(dir.x()*speed*delta_t, dir.y()*speed*delta_t, dir.z()*speed*delta_t));
}

