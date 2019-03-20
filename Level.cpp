#include "Level.h"

Level::Level(cWorld* world, Player* player) : world(world), player(player)
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

	cMatrix3d rot[6];
	double t = M_PI / 2.;
	rot[front] = cMatrix3d(cos(t), 0, sin(t), 0, 1, 0, -sin(t), 0, cos(t));
	rot[bot] = cMatrix3d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	rot[left] = cMatrix3d(cos(t), -sin(t), 0, sin(t), cos(t), 0, 0, 0, 1) * rot[0];
	rot[right] = cMatrix3d(cos(-t), -sin(-t), 0, sin(-t), cos(-t), 0, 0, 0, 1) * rot[0];
	rot[top] = cMatrix3d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	rot[back] = cMatrix3d(cos(t), 0, sin(t), 0, 1, 0, -sin(t), 0, cos(t));

	scaleFactor = 1.0;
	sideLength = 0.025 * scaleFactor;
	offset = sideLength / 2.0;
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
	audioPos[front] = cVector3d(sideLength*multiplier, 0.0, 0.0);
	audioPos[bot] = cVector3d(0.0, 0.0, -sideLength*multiplier);
	audioPos[left] = cVector3d(0.0, -sideLength*multiplier, 0.0);
	audioPos[right] = cVector3d(0.0, sideLength*multiplier, 0.0);
	audioPos[top] = cVector3d(0.0, 0.0, sideLength*multiplier);
	audioPos[back] = cVector3d(-sideLength*multiplier, 0.0, 0.0);



	for (int i = 0; i < 6; i++)
	{
		cMesh* wall = new cMesh();
			
		cCreatePlane(wall, sideLength, sideLength, pos[i], rot[i]);
		wall->createAABBCollisionDetector(0.01);
		wall->computeBTN();
		wall->m_material = cMaterial::create();
		wall->m_material->setWhite();
		wall->m_material->setUseHapticShading(true);
		wall->setStiffness(1000.0, true);
		cTexture2dPtr albedoMap = cTexture2d::create();
		albedoMap->loadFromFile(tex[i]);
		albedoMap->setWrapModeS(GL_REPEAT);
		albedoMap->setWrapModeT(GL_REPEAT);
		albedoMap->setUseMipmaps(true);
		wall->m_texture = albedoMap;
		wall->setUseTexture(true);

		walls[i] = new Wall(audio[i], audioPos[i]);
		walls[i]->mesh = wall;
		world->addChild(walls[i]->mesh);

		if (i == 5)
		{
			wall->setTransparencyLevel(0.0, true, true, true);
			wall->setUseTransparency(true);
			wall->setUseTexture(false);
		}
	}

	//--------------------------------------------------------------------------
	// SETUP AUDIO
	//--------------------------------------------------------------------------

	// create an audio device to play sounds
	audioDevice = new cAudioDevice();
	audioDevice->setListenerPos(cVector3d(0.0, 0.0, 0.0));

	// create an audio buffer and load audio wave file
	audioBuffer = new cAudioBuffer();

	bool fileload;
	fileload = audioBuffer->loadFromFile("resources/music/mm_einsteins.wav");

	// check for errors
	if (!fileload)
	{
		std::cout << "Error - Sound file failed to load or initialize correctly." << std::endl;
		//close();
		//return (-1);
	}


	// create audio source
	audioSource = new cAudioSource();

	audioSource->setSourcePos(cVector3d(0.0, 0.0, 0.0));

	// assign auio buffer to audio source
	audioSource->setAudioBuffer(audioBuffer);

	// set volume
	audioSource->setGain(10.0);

	// set speed at which the audio file is played. we will modulate this with the record speed.
	audioSource->setPitch(1.0);

	// loop audio play
	audioSource->setLoop(true);

	audioSource->setPosTime(60.f);

	// start playing
	audioSource->play();
}

cVector3d Level::computeForceDueToRoom(cVector3d pos)
{
	cShapeSphere* cursor = player->cursor;
	double r = cursor->getRadius();
	double px = pos.x() + (pos.x() >= 0 ? r : -r);
	double py = pos.y() + (pos.y() >= 0 ? r : -r);
	double pz = pos.z() + (pos.z() >= 0 ? r : -r);
	
	
	double o = offset;


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

void Level::setEar(Wall* wall, double penetration)
{
	
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