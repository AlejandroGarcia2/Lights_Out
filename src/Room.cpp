#pragma once


#include "Room.h"
#include "Player.h"

using namespace chai3d;
using namespace std;

double Room::scaleFactor = 1.0;
double Room::sideLengthX = 0.025 * scaleFactor;
double Room::sideLengthY = 0.025 * scaleFactor;
double Room::sideLengthZ = 0.005 * scaleFactor;

Room::Room(cWorld* world, cAudioDevice* audioDevice, cVector3d position, bool activated[6]) : position(position)
{
	cMatrix3d rot[6];
	double t = M_PI / 2.;
	rot[front] = cMatrix3d(cos(t), 0, sin(t), 0, 1, 0, -sin(t), 0, cos(t));
	rot[left] = cMatrix3d(cos(t), -sin(t), 0, sin(t), cos(t), 0, 0, 0, 1) * rot[0];
	rot[right] = cMatrix3d(cos(-t), -sin(-t), 0, sin(-t), cos(-t), 0, 0, 0, 1) * rot[0];
	rot[top] = rot[0] * rot[0];
	rot[bot] = cMatrix3d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	rot[back] = cMatrix3d(cos(-t), 0, sin(-t), 0, 1, 0, -sin(-t), 0, cos(-t));


	double offsetX = Room::sideLengthX / 2.0;
	double offsetY = Room::sideLengthY / 2.0;
	double offsetZ = Room::sideLengthZ / 2.0;
	cVector3d pos[6];
	pos[front] = cVector3d(-offsetX, 0.0, 0.0);
	pos[bot] = cVector3d(0.0, 0.0, -offsetZ);
	pos[left] = cVector3d(0.0, -offsetY, 0.0);
	pos[right] = cVector3d(0.0, offsetY, 0.0);
	pos[top] = cVector3d(0.0, 0.0, offsetZ);
	pos[back] = cVector3d(offsetX, 0.0, 0.0);

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
	audioPos[front] = cVector3d(Room::sideLengthX*multiplier, 0.0, 0.0);
	audioPos[bot] = cVector3d(0.0, 0.0, -Room::sideLengthZ*multiplier);
	audioPos[left] = cVector3d(0.0, -Room::sideLengthY*multiplier, 0.0);
	audioPos[right] = cVector3d(0.0, Room::sideLengthY*multiplier, 0.0);
	audioPos[top] = cVector3d(0.0, 0.0, Room::sideLengthZ*multiplier);
	audioPos[back] = cVector3d(-Room::sideLengthX*multiplier, 0.0, 0.0);



	for (int i = 0; i < 6; i++)
	{
		if (activated[i])
		{
			if (i == front || i == back)
				walls[i] = new Wall(pos[i] + position, rot[i], tex[i], Room::sideLengthZ, Room::sideLengthY);
			else if (i == left || i == right)
				walls[i] = new Wall(pos[i] + position, rot[i], tex[i], Room::sideLengthZ, Room::sideLengthY);
			else
				walls[i] = new Wall(pos[i] + position, rot[i], tex[i], Room::sideLengthX, Room::sideLengthY);

			walls[i]->initAudio(audio[i], audioDevice, audioPos[i]);
			if (i == top)
				walls[i]->setVisible(false);

			world->addChild(walls[i]->mesh);
		}
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
	audioSource->play();

	setAudioPos(position);
}