#include "Player.h"
#include "Room.h"
#include "Level.h"

Player::Player(cVector3d loc, cToolCursor* cursor, cWorld* world)
{
	speed = 0.00175;
	this->tool = cursor;
	this->earUpAgainstWall = false;
}



cVector3d Player::translateThroughDevice(double delta_t)
{
	cVector3d toolPos = tool->getDeviceLocalPos();
	//Force feedback due to moving (joystick force)
	cVector3d force = cVector3d(0.0, 0.0, 0.0);
	double threshold = 0.01;
	if (toolPos.length() >= threshold)
	{
		tool->setLocalPos(tool->getLocalPos() + cVector3d(speed*toolPos.x(), speed*toolPos.y(), 0));
		cVector3d pNorm = toolPos;
		pNorm.normalize();
		force = - 500.0 * (toolPos - pNorm*threshold);
	}

	return force;
}

int Player::getRoom(Level* lvl)
{
	double x = getPhysicalPosition().x();
	double y = getPhysicalPosition().y();

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
	y -= Room::sideLengthY;
	while (y > 0)
	{
		y -= Room::sideLengthY;
		j++;
	}

	return i * 4 + j;
}

void Player::toggleEarOnWall()
{
	earUpAgainstWall = !earUpAgainstWall;
}

void Player::initAudio(std::string source, cAudioDevice* audioDevice)
{
	// create an audio buffer and load audio wave file
	wallSoundBuffer = new cAudioBuffer();

	bool loadStatus;
	loadStatus = wallSoundBuffer->loadFromFile(source);

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
		//close();
		//return (-1);
	}

	// create audio source
	wallSoundSource = new cAudioSource();

	// assign audio buffer to audio source
	wallSoundSource->setAudioBuffer(wallSoundBuffer);
	// set volume
	wallSoundSource->setGain(0.0);
	// set speed at which the audio file is played. we will modulate this with the record speed.
	wallSoundSource->setPitch(1.0);
	// loop audio play
	wallSoundSource->setLoop(true);

	wallSoundSource->setPosTime(0.f);
}

cVector3d Player::getPhysicalPosition()
{
	return tool->getLocalPos() + tool->getDeviceLocalPos();
}

cVector3d Player::getPosition()
{
	return tool->getLocalPos();
}