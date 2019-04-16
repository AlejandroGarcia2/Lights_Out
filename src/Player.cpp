#include "Player.h"
#include "Room.h"
#include "Level.h"

Player::Player(cVector3d loc, cToolCursor* cursor, cWorld* world)
{
	speed = 0.00175;
	this->tool = cursor;
	this->micUpAgainstWall = false;
	footstepsPlaying = false;
	moveClock = new cPrecisionClock();
	body = new cShapeBox(0.0025, 0.0025, 0.005);
	world->addChild(body);
	arm = new cShapeLine(body->getLocalPos(), getPosition());
	arm->m_colorPointA = cColorf(0.0, 0.0, 0.0);
	arm->m_colorPointB = cColorf(0.0, 0.0, 0.0);
	world->addChild(arm);
	micBase = new cShapeBox(0.0010, 0.0010, 0.0020);
	micBase->m_material = cMaterial::create();
	micBase->m_material->setWhite();
	micBase->setLocalPos(tool->getLocalPos() - cVector3d(0.0, 0.0, 0.0005));
	world->addChild(micBase);
	crashAudioFile.load("resources/music/crashHaptic.wav");
	prevForce = cVector3d(0.0, 0.0, 0.0);
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

cVector3d Player::handleMovement(cVector3d& movementVector, double delta_t, Level* level, double multiplier)
{
	cVector3d force = cVector3d(0.0, 0.0, 0.0);
	//Clock not started?
	if (!moveClock->on())
	{
		// But there is input?
		if (movementVector.length() > 0)
		{
			moveClock->start(true);
			prevPos = tool->getLocalPos();
			//pentag hardcoded for square rooms
			targetPos = tool->getLocalPos() + Room::sideLengthX * movementVector;
			cVector3d halfway = 0.5f * prevPos + 0.5f* targetPos;
			validMove = level->isInBoundsX(targetPos.x()) && level->isInBoundsY(targetPos.y()) && !level->collides(movementVector);
		}
	}
	// Clock is started
	else
	{
		double clockSeconds = moveClock->getCurrentTimeSeconds();
		// Clock expired
		if (validMove)
		{
			if (clockSeconds > 1.0)
			{
				movementVector = cVector3d(0.0, 0.0, 0.0);
				moveClock->stop();
				if (footstepsPlaying)
				{
					footstepsPlaying = false;
					footstepsSoundSource->stop();
				}
			}
			else
			{

				//Force feedback due to moving (opposing to movement force)
				force = -movementVector*2.5f*multiplier;

				tool->setLocalPos((1.f - clockSeconds) * prevPos + (clockSeconds)* targetPos);

				if (!footstepsPlaying)
				{
					footstepsPlaying = true;
					footstepsSoundSource->setAudioBuffer(footstepsSoundBuffer);
					footstepsSoundSource->setGain(.5f);
					footstepsSoundSource->play();
				}
			}
		}
		else
		{
			if (clockSeconds > 3.f)
			{
				movementVector = cVector3d(0.0, 0.0, 0.0);
				moveClock->stop();
				if (footstepsPlaying)
				{
					footstepsPlaying = false;
					footstepsSoundSource->stop();
				}
			}
			else
			{
				
				

				if (clockSeconds < 0.45f)
				{
					tool->setLocalPos((1.f - clockSeconds) * prevPos + (clockSeconds)* targetPos);
					force = -movementVector*1.5f*multiplier;
				}
				else if (clockSeconds > 2.55f)
				{
					double t = 3.0 - clockSeconds;
					tool->setLocalPos((1.f - t) * prevPos + (t)* targetPos);
					force = movementVector*1.5f*multiplier;
				}
				else
				{
					int mili = (int)(clockSeconds * 1000);
					force = 0.9*prevForce + 7.5f*crashAudioFile.samples[0][mili] * cVector3d(pow(-1, mili % 3), pow(-1, mili % 2), pow(-1, mili % 4));
					prevForce = force;
				}

				if (!footstepsPlaying)
				{
					footstepsPlaying = true;
					footstepsSoundSource->setAudioBuffer(footstepsThumpSoundBuffer);
					footstepsSoundSource->setGain(.5f);
					footstepsSoundSource->play();
				}
			}
		}
	}
	return force;
}


int Player::getRoom(Level* lvl, cVector3d pos)
{
	double x = pos.x();// getPhysicalPosition().x();
	double y = pos.y();// getPhysicalPosition().y();

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
	micUpAgainstWall = !micUpAgainstWall;
}

void Player::initAudio(std::string file, cAudioDevice* audioDevice)
{
	// create an audio buffer and load audio wave file
	footstepsSoundBuffer = new cAudioBuffer();
	footstepsThumpSoundBuffer = new cAudioBuffer();
	footstepsSoundSource = new cAudioSource();
	wallSoundBuffer = new cAudioBuffer();
	bool loadStatus;
	loadStatus = footstepsSoundBuffer->loadFromFile(file);

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}

	loadStatus = footstepsThumpSoundBuffer->loadFromFile("resources/music/thumpWall.wav");

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}

	loadStatus = wallSoundBuffer->loadFromFile("resources/music/thump.wav");

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

	// create audio source
	wallSoundSource = new cAudioSource();

	// assign audio buffer to audio source
	wallSoundSource->setAudioBuffer(wallSoundBuffer);
	// set volume
	wallSoundSource->setGain(2.5f);
	// set speed at which the audio file is played. we will modulate this with the record speed.
	wallSoundSource->setPitch(1.0);
	// loop audio play
	wallSoundSource->setLoop(false);

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

cVector3d Player::getProxyPosition()
{
	return getPosition() + tool->m_hapticPoint->getLocalPosProxy();
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