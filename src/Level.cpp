#pragma once

#include <iostream>
#include <fstream>
#include "Level.h"

Level::Level(cWorld* world, Player* player) : player(player)
{

	cSpotLight *spotLight = new cSpotLight(world);
	world->addChild(spotLight);
	spotLight->setEnabled(true);
	spotLight->setDir(-1.0, -1.0, -1.0);
	spotLight->setLocalPos(cVector3d(.5, .5, .5));
	spotLight->setCutOffAngleDeg(30);
	// enable this light source to generate shadows
	spotLight->setShadowMapEnabled(true);
	spotLight->m_shadowMap->setQualityHigh();

	playTime = new cPrecisionClock();
	audioDevice = new cAudioDevice();

	createMazeFromFile(world, "resources/mazes/4x4.txt");
	initAudio();
	startAudio();
}

void Level::createMazeFromFile(cWorld* world, string path)
{
	cout << "ran!" << endl;
	ifstream myReadFile;
	myReadFile.open(path);
	char output[100];
	int roomCounter = 0;


	if (myReadFile.is_open())
	{
		//read file to check which walls we need for each room
		
		bool activated[16][6];
		int muffleLevel[16];
		while (!myReadFile.eof())
		{
			myReadFile >> output;
			cout << "Room: " << roomCounter << ", " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << ", mf: " << output[4] <<  endl;
			activated[roomCounter][front] = (bool)(output[0] - '0');
			activated[roomCounter][back] = (bool)(output[1] - '0');
			activated[roomCounter][left] = (bool)(output[2] - '0');
			activated[roomCounter][right] = (bool)(output[3] - '0');
			activated[roomCounter][bot] = true;
			activated[roomCounter][top] = true;
			muffleLevel[roomCounter] = output[4] - '0';
			roomCounter++;
		}

		roomCounter = 0;
		// make the maze
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				Room* room = new Room(world, audioDevice, cVector3d(-Room::sideLengthX*i, Room::sideLengthY*j, 0.0), activated[roomCounter], muffleLevel[roomCounter]);
				if (i == 3 && j == 0)
				{
					//room->initAudio("resources/music/aprilHaptic.wav", audioDevice);
				}
				
				rooms.push_back(room);
				roomCounter++;

				
			}
		}
	}
	myReadFile.close();
}

void Level::initAudio()
{
	std::string sorc[5];
	sorc[0] = "resources/music/april1.wav";
	sorc[1] = "resources/music/april2.wav";
	sorc[2] = "resources/music/april3.wav";
	sorc[3] = "resources/music/april4.wav";
	sorc[4] = "resources/music/aprilHapticHear.wav";

	for (int i = 0; i < 5; i++)
	{
		buffers[i] = new cAudioBuffer();


		bool loadStatus;
		loadStatus = buffers[i]->loadFromFile(sorc[i]);

		// check for errors
		if (!loadStatus)
		{
			cout << "Error - Sound file failed to load or initialize correctly." << endl;
			//close();
			//return (-1);
		}

		// create audio source
		sources[i] = new cAudioSource();

		// assign audio buffer to audio source
		sources[i]->setAudioBuffer(buffers[i]);

		// set volume
		sources[i]->setGain(0.0);

		// set speed at which the audio file is played. we will modulate this with the record speed.
		sources[i]->setPitch(1.0);

		// loop audio play
		sources[i]->setLoop(true);

		sources[i]->setPosTime(0.f);

		sources[i]->setSourcePos(rooms[12]->position);

	}
	
	std::string sorc2[3];
	sorc2[0] = "resources/music/waterDroplets.wav";
	sorc2[1] = "resources/music/waterDroplets.wav";
	sorc2[2] = "resources/music/waterDroplets.wav";

	for (int i = 0; i < 3; i++)
	{
		effectBuffers[i] = new cAudioBuffer();


		bool loadStatus;
		loadStatus = effectBuffers[i]->loadFromFile(sorc2[i]);

		// check for errors
		if (!loadStatus)
		{
			cout << "Error - Sound file failed to load or initialize correctly." << endl;
			//close();
			//return (-1);
		}

		// create audio source
		effectSources[i] = new cAudioSource();

		// assign audio buffer to audio source
		effectSources[i]->setAudioBuffer(effectBuffers[i]);

		// set volume
		effectSources[i]->setGain(1.f);

		// set speed at which the audio file is played. we will modulate this with the record speed.
		effectSources[i]->setPitch(1.0);

		// loop audio play
		effectSources[i]->setLoop(true);

		effectSources[i]->setPosTime(0.f);

		if (i == 0)
			effectSources[i]->setSourcePos(rooms[3]->position);
		else if (i == 1)
			effectSources[i]->setSourcePos(rooms[4]->position);
		else
			effectSources[i]->setSourcePos(rooms[8]->position);

	}
	
}

void Level::startAudio()
{
	for (int i = 0; i < 5; i++)
		// start playing
		sources[i]->play();
	playTime->start();
	sourceTarget = 4;
	//sources[rooms[player->getRoom(this)]->muffleLevel]->setGain(10.1);
}

double Level::getInBoundsX(double x)
{
	x -= 0.5f * Room::sideLengthX;
	double levelWidth = 4 * Room::sideLengthX;
	if (x > 0)
	{
		return 0.0001;
	}
	if (x < -levelWidth)
	{
		return levelWidth = 0.0001;
	}

	return x;
}

double Level::getInBoundsY(double y)
{
	y += 0.5f * Room::sideLengthY;
	double levelHeight = 4 * Room::sideLengthY;
	if (y < 0)
	{
		return 0.0001;
	}
	if (y > levelHeight)
	{
		return levelHeight - 0.0001;
	}

	return y;
}