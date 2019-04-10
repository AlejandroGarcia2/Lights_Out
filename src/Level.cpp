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

	audioDevice = new cAudioDevice();

	createMazeFromFile(world, "resources/mazes/4x4.txt");
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
		while (!myReadFile.eof())
		{
			myReadFile >> output;
			cout << "Room: " << roomCounter << ", " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << endl;
			activated[roomCounter][front] = (bool)(output[0] - '0');
			activated[roomCounter][back] = (bool)(output[1] - '0');
			activated[roomCounter][left] = (bool)(output[2] - '0');
			activated[roomCounter][right] = (bool)(output[3] - '0');
			activated[roomCounter][bot] = true;
			activated[roomCounter][top] = true;
			roomCounter++;
		}

		roomCounter = 0;
		// make the maze
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				Room* room = new Room(world, audioDevice, cVector3d(-Room::sideLengthX*i, Room::sideLengthY*j, 0.0), activated[roomCounter]);
				if (i == 3 && j == 0)
				{
					room->initAudio("resources/music/aprilHaptic.wav", audioDevice);
				}
				
				rooms.push_back(room);
				roomCounter++;

				
			}
		}
	}
	myReadFile.close();
}
