#pragma once

#include "Wall.h"

using namespace chai3d;
using namespace std;



Wall::Wall(cVector3d pos, cMatrix3d rot, std::string tex, double w, double h)
{
	mesh = new cMesh();

	cCreatePlane(mesh, w, h, pos, rot);
	mesh->createAABBCollisionDetector(0.01);
	mesh->computeBTN();
	mesh->m_material = cMaterial::create();
	mesh->m_material->setWhite();
	mesh->m_material->setUseHapticShading(true);
	mesh->setStiffness(3000.0, true);
	//cTexture2dPtr albedoMap = cTexture2d::create();
	//albedoMap->loadFromFile(tex);
	//albedoMap->setWrapModeS(GL_REPEAT);
	//albedoMap->setWrapModeT(GL_REPEAT);
	//albedoMap->setUseMipmaps(true);
	//mesh->m_texture = albedoMap;
	//mesh->setUseTexture(true);
}

void Wall::initAudio(std::string source, cAudioDevice* audioDevice, cVector3d audioPos)
{
	this->audioPos = audioPos;
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

	setAudioPos(audioPos);
}

void Wall::setVisible(bool a)
{
	if (!a)
	{
		mesh->setTransparencyLevel(0.0, true, true, true);
		mesh->setUseTransparency(true);
		mesh->setUseTexture(false);
		active = false;
	}
	else
	{
		mesh->setTransparencyLevel(100.0, true, true, true);
		mesh->setUseTransparency(false);
		mesh->setUseTexture(true);
		active = true;
	}
}