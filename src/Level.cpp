#pragma once

#include <iostream>
#include <fstream>
#include "Level.h"

Level::Level(cWorld* world, Player* player) : player(player)
{

	spotLight = new cSpotLight(world);
	spotLight->setEnabled(true);
	spotLight->setDir(-1.0, -1.0, -1.0);
	spotLight->setLocalPos(cVector3d(0.5, 0.5, 0.5));
	spotLight->setCutOffAngleDeg(10);
	// enable this light source to generate shadows
	spotLight->setShadowMapEnabled(true);
	spotLight->m_shadowMap->setQualityHigh();
	world->addChild(spotLight);

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
			cout << "Room: " << roomCounter << ", " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << ", mf: " << output[4] << ", extra: " << output[5] << endl;
			activated[roomCounter][front] = (bool)(output[0] - '0');
			activated[roomCounter][back] = (bool)(output[1] - '0');
			activated[roomCounter][left] = (bool)(output[2] - '0');
			activated[roomCounter][right] = (bool)(output[3] - '0');
			activated[roomCounter][bot] = true;
			activated[roomCounter][top] = true;
			muffleLevel[roomCounter] = output[4] - '0';


			if (output[5] == 'd')
				deadEndRooms.push_back(roomCounter);
			if (output[5] == 'e')
				endRoom = roomCounter;

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
	sorc[0] = "resources/music/manor0.wav";
	sorc[1] = "resources/music/manor1.wav";
	sorc[2] = "resources/music/manor2.wav";
	sorc[3] = "resources/music/manor3.wav";
	sorc[4] = "resources/music/manor4.wav";

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

		sources[i]->setSourcePos(rooms[endRoom]->position);

	}
	
	std::string sorc2[10];
	for (int i = 0; i < deadEndRooms.size(); i++)
	{
		sorc2[i] = "resources/music/waterDroplets.wav";
	}

	for (int i = 0; i < deadEndRooms.size(); i++)
	{
		effectBuffers.push_back(new cAudioBuffer());

		bool loadStatus;
		loadStatus = effectBuffers[i]->loadFromFile(sorc2[i]);

		// check for errors
		if (!loadStatus)
		{
			cout << "Error - Sound file failed to load or initialize correctly." << endl;
			//close();
			//return (-1);
		}
	}

	// create audio source
	effectSource = new cAudioSource();

	// assign audio buffer to audio source
	effectSource->setAudioBuffer(effectBuffers[0]);

	// set volume
	effectSource->setGain(1.f);

	// set speed at which the audio file is played. we will modulate this with the record speed.
	effectSource->setPitch(1.0);

	// loop audio play
	effectSource->setLoop(true);

	effectSource->setPosTime(0.f);

	effectSource->setSourcePos(rooms[deadEndRooms[0]]->position);


	std::string sorc3[3];
	sorc3[0] = "resources/music/winner.wav";
	sorc3[1] = "resources/music/loser.wav";
	sorc3[2] = "resources/music/lights_out.wav";


	winBuffer = new cAudioBuffer();
	loseBuffer = new cAudioBuffer();
	lightsOutBuffer = new cAudioBuffer();


	bool loadStatus;
	loadStatus = winBuffer->loadFromFile(sorc3[0]);

	// check for errors
	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}

	loadStatus = loseBuffer->loadFromFile(sorc3[1]);

	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}

	loadStatus = lightsOutBuffer->loadFromFile(sorc3[2]);

	if (!loadStatus)
	{
		cout << "Error - Sound file failed to load or initialize correctly." << endl;
	}


	// create audio source
	winLoseSource = new cAudioSource();

	// assign audio buffer to audio source
	winLoseSource->setAudioBuffer(winBuffer);

	// set volume
	winLoseSource->setGain(1.f);

	// set speed at which the audio file is played. we will modulate this with the record speed.
	winLoseSource->setPitch(1.0);

	// loop audio play
	winLoseSource->setLoop(false);

	winLoseSource->setPosTime(0.f);

	
	
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

bool Level::isInBoundsX(double x)
{
	x -= 0.5f * Room::sideLengthX;
	double levelWidth = 4 * Room::sideLengthX;
	if (x > 0)
	{
		return false;
	}
	if (x < -levelWidth)
	{
		return false;
	}

	return true;
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

bool Level::isInBoundsY(double y)
{
	y += 0.5f * Room::sideLengthY;
	double levelHeight = 4 * Room::sideLengthY;
	if (y < 0)
	{
		return false;
	}
	if (y > levelHeight)
	{
		return false;
	}

	return true;
}

// If the player moves in movementVector direction , will he collide?
bool Level::collides(const cVector3d& movementVector)
{
	int room = player->getRoom(this, player->getPosition());
	if ((movementVector - cVector3d(1.0, 0.0, 0.0)).length() < 0.0001)
	{
		return rooms[room]->activated[back];
		
	}
	else if((movementVector - cVector3d(-1.0, 0.0, 0.0)).length() < 0.0001)
	{
		return rooms[room]->activated[front];
		
	}
	else if ((movementVector - cVector3d(0.0, 1.0, 0.0)).length() < 0.0001)
	{
		return rooms[room]->activated[right];
		
	}
	else if ((movementVector - cVector3d(0.0, -1.0, 0.0)).length() < 0.0001)
	{
		return rooms[room]->activated[left];
		
	}
}