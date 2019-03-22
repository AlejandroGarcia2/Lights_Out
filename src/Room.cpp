#pragma once

#include "Room.h"
#include "Player.h"

using namespace chai3d;
using namespace std;

Room::Room(cWorld* world, cAudioDevice* audioDevice, cVector3d position)
{
	cMatrix3d rot[6];
	double t = M_PI / 2.;
	rot[front] = cMatrix3d(cos(t), 0, sin(t), 0, 1, 0, -sin(t), 0, cos(t));
	rot[left] = cMatrix3d(cos(t), -sin(t), 0, sin(t), cos(t), 0, 0, 0, 1) * rot[0];
	rot[right] = cMatrix3d(cos(-t), -sin(-t), 0, sin(-t), cos(-t), 0, 0, 0, 1) * rot[0];
	rot[top] = cMatrix3d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	rot[bot] = cMatrix3d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	rot[back] = cMatrix3d(cos(t), 0, sin(t), 0, 1, 0, -sin(t), 0, cos(t));

	Wall::scaleFactor = 1.0;
	Wall::sideLength = 0.025 * Wall::scaleFactor;
	double offset = Wall::sideLength / 2.0;
	cVector3d pos[6];
	pos[front] = cVector3d(-offset, 0.0, 0.0);
	pos[bot] = cVector3d(0.0, 0.0, -offset);
	pos[left] = cVector3d(0.0, -offset, 0.0);
	pos[right] = cVector3d(0.0, offset, 0.0);
	pos[top] = cVector3d(0.0, 0.0, offset);
	pos[back] = cVector3d(offset, 0.0, 0.0);

	std::string audio[6];
	audio[front] = "resources/music/windMono.wav";
	audio[bot] = "resources/music/mm_einsteinsBaby.wav";
	audio[left] = "resources/music/windMono.wav";
	audio[right] = "resources/music/m_einsteins.wav";
	audio[top] = "resources/music/mm_einsteinsSteps.wav";
	audio[back] = "resources/music/mm_einsteinsWind.wav";

	std::string tex[6];
	tex[front] = "resources/front.png";
	tex[bot] = "resources/bot.png";
	tex[left] = "resources/left.png";
	tex[right] = "resources/right.png";
	tex[top] = "resources/top.png";
	tex[back] = "resources/front.png";

	cVector3d audioPos[6];
	//multiplier to easily change exaggeration of position of soynds coming from walls
	double multiplier = 3.f;
	audioPos[front] = cVector3d(Wall::sideLength*multiplier, 0.0, 0.0);
	audioPos[bot] = cVector3d(0.0, 0.0, -Wall::sideLength*multiplier);
	audioPos[left] = cVector3d(0.0, -Wall::sideLength*multiplier, 0.0);
	audioPos[right] = cVector3d(0.0, Wall::sideLength*multiplier, 0.0);
	audioPos[top] = cVector3d(0.0, 0.0, Wall::sideLength*multiplier);
	audioPos[back] = cVector3d(-Wall::sideLength*multiplier, 0.0, 0.0);



	for (int i = 0; i < 6; i++)
	{
		walls[i] = new Wall(pos[i] + position, rot[i], tex[i]);
		walls[i]->initAudio(audio[i], audioDevice, audioPos[i]);
		world->addChild(walls[i]->mesh);
	}
}

void Room::initAudio(std::string source, cAudioDevice* audioDevice)
{
	// create an audio buffer and load audio wave file
	audioBuffer = new cAudioBuffer();


	bool loadStatus;
	loadStatus = audioBuffer->loadFromFile(source);

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
		//close();
		//return (-1);
	}

	// create audio source
	audioSource = new cAudioSource();

	// assign audio buffer to audio source
	audioSource->setAudioBuffer(audioBuffer);

	// set volume
	audioSource->setGain(0.0);

	// set speed at which the audio file is played. we will modulate this with the record speed.
	audioSource->setPitch(1.0);

	// loop audio play
	audioSource->setLoop(true);

	audioSource->setPosTime(60.f);

	// start playing
	//audioSource->play();

	setAudioPos(position);
}


cVector3d Room::computeForceDueToRoom(cVector3d pos, Player* player)
{
	cShapeSphere* cursor = player->cursor;

	
	double r = cursor->getRadius();
	double px = pos.x() + (pos.x() >= 0 ? r : -r);
	double py = pos.y() + (pos.y() >= 0 ? r : -r);
	double pz = pos.z() + (pos.z() >= 0 ? r : -r);


	double o = Wall::sideLength / 2.0;


	double K = 2500; // N/m
	cVector3d force = cVector3d(0.0, 0.0, 0.0);
	bool in = px < -o || px > o || py < -o || py > o || pz < -o || pz > o;

	// went inside wall
	if (in && !player->insideWall)
	{
		player->insideWall = true;
		if (px < -o) setEar(walls[back], px + o);
		else if (px > o) setEar(walls[front], px - o);
		if (py < -o) setEar(walls[left], py + o);
		else if (py > o) setEar(walls[right], py - o);
		if (pz < -o) setEar(walls[bot], pz + o);
		else if (pz > o) setEar(walls[top], pz - o);
	}
	// continues to be inside wall
	else if (in && player->insideWall)
	{
		if (px < -o)
		{
			force += cVector3d(-K * (px + o), 0.0, 0.0);
			cursor->setLocalPos(cVector3d(-o + r, cursor->getLocalPos().y(), cursor->getLocalPos().z()));
			setEar(walls[back], -o - px);
		}
		else if (px > o)
		{
			force += cVector3d(-K * (px - o), 0.0, 0.0);
			cursor->setLocalPos(cVector3d(o - r, cursor->getLocalPos().y(), cursor->getLocalPos().z()));
			setEar(walls[front], px - o);
		}
		if (py < -o)
		{
			force += cVector3d(0.0, -K * (py + o), 0.0);
			cursor->setLocalPos(cVector3d(cursor->getLocalPos().x(), -o + r, cursor->getLocalPos().z()));
			setEar(walls[left], -o - py);
		}
		else if (py > o)
		{
			force += cVector3d(0.0, -K * (py - o), 0.0);
			cursor->setLocalPos(cVector3d(cursor->getLocalPos().x(), o - r, cursor->getLocalPos().z()));
			setEar(walls[right], py - o);
		}
		if (pz < -o)
		{
			force += cVector3d(0.0, 0.0, -K * (pz + o));
			cursor->setLocalPos(cVector3d(cursor->getLocalPos().x(), cursor->getLocalPos().y(), -o + r));
			setEar(walls[bot], -o - pz);
		}
		else if (pz > o)
		{
			force += cVector3d(0.0, 0.0, -K * (pz - o));
			cursor->setLocalPos(cVector3d(cursor->getLocalPos().x(), cursor->getLocalPos().y(), o - r));
			setEar(walls[top], pz - o);
		}
	}
	// went outside wall
	else  if (!in && player->insideWall)
	{
		player->insideWall = false;
		setEar(NULL, 0);

	}
	// continues to be outside wall
	else
	{
	}

	return force;
}

void Room::setEar(Wall* wall, double penetration)
{
	return;
	// 0.75cm penetration seemed a good threshold
	if (penetration > 0.005) penetration = 0.005;
	// Gain(volume) of the wall goes from 0 at penetration == 0.0, to 10 at penetration == 0.005, quadratically
	double wallGain = 400000.f * penetration * penetration;
	// Gain(volume) of the room goes from 10 at penetration == 0.0, to 0.05 at penetration == 0.005
	double roomGain = 10.05f - wallGain;

	for (int i = 0; i < 6; i++)
	{
		if (wall == walls[i])
		{
			walls[i]->audioSource->setGain(wallGain);
			audioSource->setGain(roomGain);
		}
		else
		{
			walls[i]->audioSource->setGain(0.0);
		}
	}
	// Took ear off the wall
	if (wall == NULL)
		audioSource->setGain(10.0f);
}