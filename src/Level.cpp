#pragma once

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

	Room* room = new Room(world, audioDevice, cVector3d(0.0, 0.0, 0.0));
	room->initAudio("resources/music/CloudSymphony.wav", audioDevice);
	player->room = room;
}
