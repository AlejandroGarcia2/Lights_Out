#include "Wall.h"

using namespace chai3d;
using namespace std;

Wall::Wall(std::string source) : source(source)
{
	
}

void Wall::initAudio(cAudioDevice* audioDevice)
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

	// assign auio buffer to audio source
	audioSource->setAudioBuffer(audioBuffer);

	// set volume
	audioSource->setGain(0.0);

	// set speed at which the audio file is played. we will modulate this with the record speed.
	audioSource->setPitch(1.0);

	audioSource->setSourcePos(cVector3d(0.0, -5.0, 0.0));

	// loop audio play
	audioSource->setLoop(true);

	audioSource->setPosTime(60.f);

	// start playing
	audioSource->play();
}