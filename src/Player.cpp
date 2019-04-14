#include "Player.h"
#include "Room.h"
#include "Level.h"

Player::Player(cVector3d loc, cToolCursor* cursor, cWorld* world)
{
	speed = 0.00175;
	this->tool = cursor;
	this->earUpAgainstWall = false;
	footstepsPlaying = false;
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

		if (!footstepsPlaying)
		{
			footstepsPlaying = true;
			footstepsSoundSource->setGain(.5f);
			footstepsSoundSource->setSourcePos(tool->getLocalPos());
			footstepsSoundSource->play();
		}

		cVector3d pNorm = toolPos;
		pNorm.normalize();
		force = - 750.0 * (toolPos - pNorm*threshold);
	}
	else
	{
		if (footstepsPlaying)
		{
			footstepsPlaying = false;
			footstepsSoundSource->stop();
		}
	}

	return force;
}

int Player::getRoom(Level* lvl)
{
	double x = getPhysicalPosition().x();
	double y = getPhysicalPosition().y();

	x = lvl->getInBoundsX(x);
	y = lvl->getInBoundsY(y);

	//x -= Room::sideLengthX / 2.;
	//y += Room::sideLengthY / 2.;

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

void Player::initAudio(std::string file, cAudioDevice* audioDevice)
{
	// create an audio buffer and load audio wave file
	footstepsSoundBuffer = new cAudioBuffer();
	footstepsSoundSource = new cAudioSource();

	bool loadStatus;
	loadStatus = footstepsSoundBuffer->loadFromFile(file);

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}

	// create audio source
	footstepsSoundSource = new cAudioSource();

	// assign audio buffer to audio source
	footstepsSoundSource->setAudioBuffer(footstepsSoundBuffer);
	// set volume
	footstepsSoundSource->setGain(0.1f);
	// set speed at which the audio file is played. we will modulate this with the record speed.
	footstepsSoundSource->setPitch(1.0);
	// loop audio play
	footstepsSoundSource->setLoop(true);

	footstepsSoundSource->setPosTime(0.f);
}

cVector3d Player::getPhysicalPosition()
{
	return tool->getLocalPos() + tool->getDeviceLocalPos();
}

cVector3d Player::getPosition()
{
	return tool->getLocalPos();
}

void Player::handleFootsteps(cAudioDevice* ad)
{
	if (startPlayingFootsteps)
	{
		startPlayingFootsteps = false;
		footstepsSoundSource->setGain(10.f);
	}
	if (stopPlayingFootsteps)
	{
		startPlayingFootsteps = false;
		footstepsSoundSource->setGain(0.0);
	}
}