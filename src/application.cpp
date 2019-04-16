
//==============================================================================
/*
\author    Your Name
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "MyProxyAlgorithm.h"
#include "MyMaterial.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
#include "Player.h"
#include "Level.h"
#include "AudioFile.h"

//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
C_STEREO_DISABLED:            Stereo is disabled
C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight *light;

Player* player;
Level* level;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a label to display the rates [Hz] at which the simulation is running
cLabel* labelRates;
cLabel* labelGameTimer;

// a small sphere (cursor) representing the haptic device 
cShapeSphere* cursor;

// a small sphere (cursor) representing the haptic device 
cToolCursor* tool;

// a pointer to the custom proxy rendering algorithm inside the tool
MyProxyAlgorithm* proxyAlgorithm;


// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = false;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

int movingRight = 0;
int movingLeft = 0;
int movingBackwards = 0;
int movingForwards = 0;
cVector3d movementVector;
bool hint = false;
double multiplier = 1.0;


AudioFile<double> audioFile;
AudioFile<double> footstepAudioFile;
AudioFile<double> thumpAudioFile;

cPrecisionClock* gameTimer;
int secondsAllowed = 180;
bool won = false;
std::string endText;

cMesh* cover;
bool isCovered;

//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// this function renders the scene
void updateGraphics(double);

// this function contains the main haptics simulation loop
void updateHaptics(void);

void computeGain();
void computeMuffleness();
void handleMuffleLevelInterpolation();

// this function closes the application
void close(void);


//==============================================================================
/*
TEMPLATE:    application.cpp

Description of your application.
*/
//==============================================================================

int main(int argc, char* argv[])
{
	//wavData = readWav("resources/music/aprilHaptic.wav");
	//for (int i = 0; i < wavData.size(); i++)
	//{
	//	cout << wavData[i] << endl;
	//}
	//cout << wavData.size();

	audioFile.load("resources/music/manorHaptic.wav");
	footstepAudioFile.load("resources/music/footsteopsHaptic.wav");
	thumpAudioFile.load("resources/music/thumpHaptic.wav");

	/*for (int i = 0; i < numSamples; i++)
	{
	double currentSample = audioFile.samples[channel][i];
	cout << "currentSample:" << currentSample << endl;
	}*/

	//--------------------------------------------------------------------------
	// INITIALIZATION
	//--------------------------------------------------------------------------

	cout << endl;
	cout << "-----------------------------------" << endl;
	cout << "CHAI3D" << endl;
	cout << "-----------------------------------" << endl << endl << endl;
	cout << "Keyboard Options:" << endl << endl;
	cout << "[f] - Enable/Disable full screen mode" << endl;
	cout << "[m] - Enable/Disable vertical mirroring" << endl;
	cout << "[q] - Exit application" << endl;
	cout << endl << endl;


	//--------------------------------------------------------------------------
	// OPENGL - WINDOW DISPLAY
	//--------------------------------------------------------------------------

	// initialize GLFW library
	if (!glfwInit())
	{
		cout << "failed initialization" << endl;
		cSleepMs(1000);
		return 1;
	}

	// set error callback
	glfwSetErrorCallback(errorCallback);

	// compute desired size of window
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int w = 0.8 * mode->height;
	int h = 0.5 * mode->height;
	int x = 0.5 * (mode->width - w);
	int y = 0.5 * (mode->height - h);

	// set OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// set active stereo mode
	if (stereoMode == C_STEREO_ACTIVE)
	{
		glfwWindowHint(GLFW_STEREO, GL_TRUE);
	}
	else
	{
		glfwWindowHint(GLFW_STEREO, GL_FALSE);
	}

	// create display context
	window = glfwCreateWindow(w, h, "Lights Out", NULL, NULL);
	if (!window)
	{
		cout << "failed to create window" << endl;
		cSleepMs(1000);
		glfwTerminate();
		return 1;
	}

	// get width and height of window
	glfwGetWindowSize(window, &width, &height);

	// set position of window
	glfwSetWindowPos(window, x, y);

	// set key callback
	glfwSetKeyCallback(window, keyCallback);

	// set resize callback
	glfwSetWindowSizeCallback(window, windowSizeCallback);

	// set current display context
	glfwMakeContextCurrent(window);

	// sets the swap interval for the current display context
	glfwSwapInterval(swapInterval);

#ifdef GLEW_VERSION
	// initialize GLEW library
	if (glewInit() != GLEW_OK)
	{
		cout << "failed to initialize GLEW library" << endl;
		glfwTerminate();
		return 1;
	}
#endif


	//--------------------------------------------------------------------------
	// WORLD - CAMERA - LIGHTING
	//--------------------------------------------------------------------------

	// create a new world.
	world = new cWorld();

	// set the background color of the environment
	world->m_backgroundColor.setBlack();

	// create a camera and insert it into the virtual world
	camera = new cCamera(world);
	world->addChild(camera);

	// position and orient the camera
	camera->set(cVector3d(0.5, 0.0, 0.0),    // camera position (eye)
		cVector3d(0.0, 0.0, 0.0),    // look at position (target)
		cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

									 // set the near and far clipping planes of the camera
	camera->setClippingPlanes(0.01, 10.0);

	// set stereo mode
	camera->setStereoMode(stereoMode);

	// set stereo eye separation and focal length (applies only if stereo is enabled)
	camera->setStereoEyeSeparation(0.01);
	camera->setStereoFocalLength(0.5);

	// set vertical mirrored display mode
	camera->setMirrorVertical(mirroredDisplay);

	// create a directional light source
	//light = new cDirectionalLight(world);

	// insert light source inside world
	//world->addChild(light);

	// enable light source
	//light->setEnabled(true);

	// define direction of light beam
	//light->setDir(-1.0, 0.0, 0.0);

	// use a point avatar for this scene
	double toolRadius = 0.0;

	// create a sphere (cursor) to represent the haptic device
	cursor = new cShapeSphere(0.001);

	// insert cursor inside world
	//world->addChild(cursor);

	cover = new cMesh();

	cCreatePlane(cover, 1.0, 1.0, cVector3d(0.0, 0.0, Room::sideLengthZ));
	cover->m_material = cMaterial::create();
	cover->m_material->setBlack();
	cover->setUseTransparency(true);
	cover->m_material->setTransparencyLevel(0.0);
	world->addChild(cover);


	//--------------------------------------------------------------------------
	// HAPTIC DEVICE
	//--------------------------------------------------------------------------

	// create a haptic device handler
	handler = new cHapticDeviceHandler();

	// get a handle to the first haptic device
	handler->getDevice(hapticDevice, 0);

	// if the device has a gripper, enable the gripper to simulate a user switch
	hapticDevice->setEnableGripperUserSwitch(true);

	/////////////////////////////////////////////////////////////
	tool = new cToolCursor(world);
	tool->m_material = cMaterial::create();
	tool->m_material->setBlack();
	tool->m_material->setShininess(0.5);
	world->addChild(tool);
	//tool->translate(0.0, 0.1, 0.0);


	// [CPSC.86] replace the tool's proxy rendering algorithm with our own
	proxyAlgorithm = new MyProxyAlgorithm;
	delete tool->m_hapticPoint->m_algorithmFingerProxy;
	tool->m_hapticPoint->m_algorithmFingerProxy = proxyAlgorithm;

	tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

	tool->setRadius(0.001, toolRadius);

	tool->setHapticDevice(hapticDevice);

	tool->setWaitForSmallForce(true);

	tool->start();
	/////////////////////////////////////////////////////////////

	/*
	// open a connection to haptic device
	hapticDevice->open();

	// calibrate device (if necessary)
	hapticDevice->calibrate();

	// retrieve information about the current haptic device
	cHapticDeviceInfo info = hapticDevice->getSpecifications();

	// display a reference frame if haptic device supports orientations
	if (info.m_sensedRotation == true)
	{
	// display reference frame
	cursor->setShowFrame(true);

	// set the size of the reference frame
	cursor->setFrameSize(0.05);
	}

	// if the device has a gripper, enable the gripper to simulate a user switch
	hapticDevice->setEnableGripperUserSwitch(true);
	*/

	//--------------------------------------------------------------------------
	// PLAYER
	//--------------------------------------------------------------------------

	player = new Player(cVector3d(0.0, 0.0, 0.0), tool, world);

	//--------------------------------------------------------------------------
	// Level
	//--------------------------------------------------------------------------

	level = new Level(world, player);
	player->initAudio("resources/music/footsteops.wav", level->audioDevice);

	//--------------------------------------------------------------------------
	// WIDGETS
	//--------------------------------------------------------------------------

	// create a font
	cFontPtr font = NEW_CFONTCALIBRI20();

	// create a label to display the haptic and graphic rates of the simulation
	labelRates = new cLabel(font);
	labelRates->m_fontColor.setWhite();
	//camera->m_frontLayer->addChild(labelRates);

	cFontPtr font2 = NEW_CFONTCALIBRI144();

	labelGameTimer = new cLabel(font2);
	labelGameTimer->m_fontColor.setWhite();
	camera->m_frontLayer->addChild(labelGameTimer);

	//--------------------------------------------------------------------------
	// START SIMULATION
	//--------------------------------------------------------------------------

	// create a thread which starts the main haptics rendering loop
	hapticsThread = new cThread();
	hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

	// setup callback when application exits
	atexit(close);


	//--------------------------------------------------------------------------
	// MAIN GRAPHIC LOOP
	//--------------------------------------------------------------------------

	// call window size callback at initialization
	windowSizeCallback(window, width, height);

	cPrecisionClock timer;
	timer.start();
	double t_previous = timer.getCurrentTimeSeconds();

	gameTimer = new cPrecisionClock();
	// main graphic loop
	while (!glfwWindowShouldClose(window))
	{
		
		// get width and height of window
		glfwGetWindowSize(window, &width, &height);

		// calculate elapsed time
		double t_current = timer.getCurrentTimeSeconds();
		double delta_t = t_current - t_previous;
		t_previous = t_current;

		// render graphics
		updateGraphics(delta_t);

		// swap buffers
		glfwSwapBuffers(window);

		// process events
		glfwPollEvents();

		// signal frequency counter
		freqCounterGraphics.signal(1);
	}

	// close window
	glfwDestroyWindow(window);

	// terminate GLFW library
	glfwTerminate();

	// exit
	return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
	// update window size
	width = a_width;
	height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
	cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{

	if (a_action == GLFW_PRESS)
	{
		// option - exit
		if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
		{
			glfwSetWindowShouldClose(a_window, GLFW_TRUE);
		}

		// option - toggle fullscreen
		else if (a_key == GLFW_KEY_F)
		{
			// toggle state variable
			fullscreen = !fullscreen;

			// get handle to monitor
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();

			// get information about monitor
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			// set fullscreen or window mode
			if (fullscreen)
			{
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				glfwSwapInterval(swapInterval);
			}
			else
			{
				int w = 0.8 * mode->height;
				int h = 0.5 * mode->height;
				int x = 0.5 * (mode->width - w);
				int y = 0.5 * (mode->height - h);
				glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
				glfwSwapInterval(swapInterval);
			}
		}

		// option - toggle vertical mirroring
		else if (a_key == GLFW_KEY_M)
		{
			mirroredDisplay = !mirroredDisplay;
			camera->setMirrorVertical(mirroredDisplay);
		}
		else if (a_key == GLFW_KEY_D || a_key == GLFW_KEY_RIGHT)
		{
			if (!player->moveClock->on())
			{
				cout << "move" << endl;
				movementVector = cVector3d(0.0, 1.0, 0.0);
				movingRight = 1;
				multiplier = 1.f;
			}

		}
		else if (a_key == GLFW_KEY_A || a_key == GLFW_KEY_LEFT)
		{
			if (!player->moveClock->on())
			{
				cout << "move" << endl;
				movementVector = cVector3d(0.0, -1.0, 0.0);
				movingLeft = -1;
				multiplier = 1.f;
			}

		}
		else if (a_key == GLFW_KEY_W || a_key == GLFW_KEY_UP)
		{
			if (!player->moveClock->on())
			{
				cout << "move" << endl;
				movementVector = cVector3d(-1.0, 0.0, 0.0);
				movingForwards = -1;
				multiplier = 2.f;
			}
		}
		else if (a_key == GLFW_KEY_S || a_key == GLFW_KEY_DOWN)
		{
			cout << "move" << endl;
			if (!player->moveClock->on())
			{
				movementVector = cVector3d(1.0, 0.0, 0.0);
				movingBackwards = 1;
				multiplier = 2.f;
			}

		}
		else if (a_key == GLFW_KEY_SPACE)
		{
			hint = true;
		}
		else if (a_key == GLFW_KEY_ENTER)
		{
			if (!gameTimer->on())
				gameTimer->start();
			level->winLoseSource->stop();
			level->spotLight->setEnabled(!level->spotLight->getEnabled());
			isCovered = !isCovered;
			if (isCovered)
				cover->setTransparencyLevel(100.f);
			else
				cover->setTransparencyLevel(0.f);
			level->winLoseSource->setAudioBuffer(level->lightsOutBuffer);
			level->winLoseSource->setGain(5.f);
			level->winLoseSource->play();
		}
		else if (a_key == GLFW_KEY_O)
		{
			won = false;
			if (gameTimer->on())
			{
				gameTimer->reset();
				gameTimer->stop();
			}
			isCovered = false;
			level->spotLight->setEnabled(true);
			cover->setTransparencyLevel(0.f);
			player->tool->setLocalPos(cVector3d(0.0, 0.0, 0.0));
		}
		else if (a_key == GLFW_KEY_P)
		{
			player->tool->setHapticEnabled(true);
		}
	}
	else if (a_action == GLFW_RELEASE)
	{
		if (a_key == GLFW_KEY_D || a_key == GLFW_KEY_RIGHT)
		{
			//movingRight = 0;
		}
		else if (a_key == GLFW_KEY_A || a_key == GLFW_KEY_LEFT)
		{
			//movingLeft = 0;
		}
		else if (a_key == GLFW_KEY_W || a_key == GLFW_KEY_UP)
		{
			//movingForwards = 0;
		}
		else if (a_key == GLFW_KEY_S || a_key == GLFW_KEY_DOWN)
		{
			//movingBackwards = 0;
		}
		else if (a_key == GLFW_KEY_SPACE)
		{
			level->sourceTarget = level->rooms[player->getRoom(level, player->getPosition())]->muffleLevel;
			hint = false;
		}
	}
}

//------------------------------------------------------------------------------

void close(void)
{
	// stop the simulation
	simulationRunning = false;

	// wait for graphics and haptics loops to terminate
	while (!simulationFinished) { cSleepMs(100); }

	// close haptic device
	hapticDevice->close();

	// delete resources
	delete hapticsThread;
	delete world;
	delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(double delta_t)
{
	/////////////////////////////////////////////////////////////////////
	// UPDATE WIDGETS
	/////////////////////////////////////////////////////////////////////
	std::string debug = cStr(proxyAlgorithm->m_debugValue); +" " + cStr(proxyAlgorithm->m_debugVector.x()) +
		" " + cStr(proxyAlgorithm->m_debugVector.y()) + " " + cStr(proxyAlgorithm->m_debugVector.z());

	// update haptic and graphic rate data
	labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
		cStr(freqCounterHaptics.getFrequency(), 0) + " Hz debug" + debug);

	// update position of label
	labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);

	int timeLeft = secondsAllowed - (int)gameTimer->getCurrentTimeSeconds();
	int minsLeft = timeLeft / 60;
	if (minsLeft < 0) minsLeft = 0;
	int secsLeft = timeLeft % 60;
	if(secsLeft < 0) secsLeft = 0;
	string extra = secsLeft < 10 ? "0" : "";
	if (minsLeft == 3)
	{
		labelGameTimer->setText("Press Enter to start");
	}
	else if (!won)
		labelGameTimer->setText(to_string(minsLeft) + ":" + extra + to_string(secsLeft));
	else
		labelGameTimer->setText(endText);
	labelGameTimer->setLocalPos((int)(0.5 * (width - labelGameTimer->getWidth())), height - 160);

	if (minsLeft == 0 && secsLeft == 9 && !won)
	{
		level->winLoseSource->stop();
		level->winLoseSource->setAudioBuffer(level->loseBuffer);
		level->winLoseSource->setGain(0.5f);
		level->winLoseSource->play();
	}


	/////////////////////////////////////////////////////////////////////
	// RENDER SCENE
	/////////////////////////////////////////////////////////////////////

	// update shadow maps (if any)
	world->updateShadowMaps(false, mirroredDisplay);

	// render world
	camera->renderView(width, height);

	// wait until all GL commands are completed
	glFinish();

	// check for any OpenGL errors
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) cout << "Error:  %s\n" << gluErrorString(err);

	/////////////////////////////////////////////////////////////////////
	// PLAYER INPUT
	/////////////////////////////////////////////////////////////////////

	//std::cout << "forward: " << movingForwards << "left: " << movingLeft << "down: " << movingBackwards << "right: " << movingRight << std::endl;

	camera->set(player->getPosition() + cVector3d(0.08, 0.0, 0.08),    // camera position (eye)
		player->getPosition(),    // look at position (target)
		cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector
									 //camera->attachAudioDevice(level->audioDevice);
									 //level->audioDevice->setListenerPos(cVector3d(50.0, 0.0, 0.0));

	player->body->setLocalPos(tool->getLocalPos() + cVector3d(0.0, 0.0, 0.0025));
	player->arm->m_pointA = player->getPosition() + cVector3d(0.0, 0.00125, 0.0025);
	player->arm->m_pointB = player->getProxyPosition();
	//level->spotLight->setLocalPos(player->getPosition() + cVector3d(0.0, 0.0, 0.05));


	computeGain();

	computeMuffleness();

	handleMuffleLevelInterpolation();

	player->footstepsSoundSource->setSourcePos(player->getPosition());
	player->wallSoundSource->setSourcePos(player->getProxyPosition());
	level->audioDevice->setListenerPos(player->getProxyPosition());

}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
	//timestamp last frame
	double t0;
	//total time spent in scene 3
	double delta_t;

	cPrecisionClock clock;

	// simulation in now running
	simulationRunning = true;
	simulationFinished = false;

	t0 = clock.getCPUTimeSeconds();
	// main haptic simulation loop

	double toolR = 0.001;
	double footstepsToolR = 0.001;
	while (simulationRunning)
	{
		delta_t = clock.getCPUTimeSeconds() - t0;

		/////////////////////////////////////////////////////////////////////
		// READ HAPTIC DEVICE
		/////////////////////////////////////////////////////////////////////

		// read position 
		cVector3d position;
		hapticDevice->getPosition(position);


		// read orientation 
		cMatrix3d rotation;
		hapticDevice->getRotation(rotation);

		// read user-switch status (button 0)
		bool button = false;
		hapticDevice->getUserSwitch(0, button);


		world->computeGlobalPositions();


		/////////////////////////////////////////////////////////////////////
		// UPDATE 3D CURSOR MODEL
		/////////////////////////////////////////////////////////////////////

		tool->updateFromDevice();
		player->micBase->setLocalPos(player->getProxyPosition() - cVector3d(0.0, 0.0, 0.0005));

		/////////////////////////////////////////////////////////////////////
		// COMPUTE FORCES
		/////////////////////////////////////////////////////////////////////

		tool->computeInteractionForces();

		int milisecond = (int)(level->playTime->getCurrentTimeSeconds() * 1000);

		cVector3d force(0, 0, 0);
		cVector3d torque(0, 0, 0);
		double gripperForce = 0.0;

		toolR = 0.995*toolR + 0.05*(audioFile.samples[0][milisecond]);


		/////////////////////////////////////////////////////////////////////
		// MOVEMENT
		/////////////////////////////////////////////////////////////////////

		//player->translate(cVector3d(movingForwards + movingBackwards, movingRight + movingLeft, 0), delta_t);
		cVector3d f(toolR, toolR, toolR);

		cVector3d normal = tool->m_hapticPoint->getCollisionEvent(0)->m_globalNormal;
		normal.normalize();

		if (normal.length() > 0.0001)
		{
		
			double a = acos(cDot(normal, cVector3d(0.0, 1.0, 0.0)));
			double b = acos(cDot(normal, cVector3d(0.0, -1.0, 0.0)));
			double c = acos(cDot(normal, cVector3d(1.0, 0.0, 0.0)));
			double d = acos(cDot(normal, cVector3d(-1.0, 0.0, 0.0)));
			if (a < M_PI / 20 || b < M_PI / 20)
			{
				tool->addDeviceLocalForce(50 * (toolR)*normal - 10 * tool->getDeviceLocalLinVel());
				if (!player->micUpAgainstWall)
				{
					player->toggleEarOnWall();
					player->wallSoundSource->play();
				}
			}
			else if (c < M_PI / 20 || d < M_PI / 20)
			{
				tool->addDeviceLocalForce(100 * (toolR)*normal - 10 * tool->getDeviceLocalLinVel());
				if (!player->micUpAgainstWall)
				{
					player->toggleEarOnWall();
					player->wallSoundSource->play();
				}
			}
		}
		else
		{
			if (player->micUpAgainstWall)
			{
				player->toggleEarOnWall();
				player->wallSoundSource->stop();
			}
		}

		tool->addDeviceLocalForce(player->handleMovement(movementVector, delta_t, level, multiplier));

		tool->addDeviceLocalForce(-tool->getDeviceLocalPos() * 60.f);
		/////////////////////////////////////////////////////////////////////
		// APPLY FORCES
		/////////////////////////////////////////////////////////////////////

		tool->applyToDevice();

		// signal frequency counter
		freqCounterHaptics.signal(1);

		t0 = clock.getCPUTimeSeconds();
	}

	// exit haptics thread
	simulationFinished = true;
}

void computeGain()
{
	
	if (!hint)
	{
		int curRoom = player->getRoom(level, player->getPhysicalPosition());
		if (curRoom != -1)
		{
			level->audioDevice->setListenerPos(player->getPhysicalPosition());
			double lvl = 4.f * Room::sideLengthX - (player->getProxyPosition() - level->rooms[level->endRoom]->position).length();
			if (lvl < 0) lvl = 0.f;
			level->sourceTargetValue = lvl;
			if ((player->getProxyPosition() - level->rooms[level->endRoom]->position).length() < 0.0175)
			{
				won = true;
				level->winLoseSource->stop();
				level->winLoseSource->setAudioBuffer(level->winBuffer);
				level->winLoseSource->setGain(0.5f);
				level->winLoseSource->play();
				endText = "You win!";
			}
		}
		else
			level->sourceTargetValue = 0.0;
		/*
		for (int i = 0; i < 5; i++)
		{
		level->sources[i]->setGain(0.0f);
		}
		level->sources[mf]->setGain(lvl);*/

	}
	else
	{
		level->audioDevice->setListenerPos(level->sources[0]->getSourcePos());
		level->sourceTarget = 0;
		level->sourceTargetValue = 4 * Room::sideLengthX;

	}
}

void computeMuffleness()
{
	int r = player->getRoom(level, player->getPosition());

	//Player has changed room since last frame
	if (player->room != r)
	{
		player->room = r;
		if (r != -1)
		{
			int muffleTarget = level->rooms[r]->muffleLevel;
			level->sourceTarget = muffleTarget;
		}
		for (int i = 0; i < level->deadEndRooms.size(); i++)
		{
			level->effectSource->stop();
		}
		for (int i = 0; i < level->deadEndRooms.size(); i++)
		{
			if (r == level->deadEndRooms[i])
			{
				level->effectSource->setAudioBuffer(level->effectBuffers[i]);
				level->effectSource->setSourcePos(level->rooms[r]->position);
				level->effectSource->play();
			}

		}
		/*if (r == level->deadEndRooms[0])
		{
			level->effectSources[0]->play();
		}
		else if (r == level->deadEndRooms[1])
		{
			level->effectSources[1]->play();
		}
		else if (r == level->deadEndRooms[2])
		{
			level->effectSources[2]->play();
		}
		else if (r == level->deadEndRooms[3])
		{
			level->effectSources[3]->play();
		}
		else if (r == level->deadEndRooms[4])
		{
			level->effectSources[4]->play();
		}
		else
		{
			for (int i = 0; i < 5; i++)
			{
				level->effectSources[i]->stop();
			}
		}*/
	}
}

void handleMuffleLevelInterpolation()
{
	//Update sound every frame so that level->sources[level->sourceTarget].gain goes towards 1 and all others go towards 0
	// This is to stop 'pop' effect from suddently switching between muffle level audio sources
	for (int i = 0; i < 5; i++)
	{
		double curGain = level->sources[i]->getGain();
		// if i is the target audio source and it's not yet at it's target value
		if (i == level->sourceTarget)
		{
			double toGain = level->sourceTargetValue - curGain;
			if (toGain > 0.05)
				level->sources[i]->setGain(curGain + 0.05);
			else
				level->sources[i]->setGain(level->sourceTargetValue);
		}
		else
		{
			if (curGain > 0.05)
				level->sources[i]->setGain(curGain - 0.05);
		}
	}
}
