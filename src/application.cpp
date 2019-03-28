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
int width  = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

int movingRight = 0;
int movingLeft = 0;
int movingBackwards = 0;
int movingForwards = 0;
bool hint = false;


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
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
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
    camera->set( cVector3d (0.5, 0.0, 0.0),    // camera position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // look at position (target)
                 cVector3d (0.0, 0.0, 1.0));   // direction of the (up) vector

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
    light = new cDirectionalLight(world);

    // insert light source inside world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define direction of light beam
    light->setDir(-1.0, 0.0, 0.0); 

	// use a point avatar for this scene
	double toolRadius = 0.0;

    // create a sphere (cursor) to represent the haptic device
    cursor = new cShapeSphere(0.001);

    // insert cursor inside world
    //world->addChild(cursor);


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

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI20();
    
    // create a label to display the haptic and graphic rates of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setWhite();
    camera->m_frontLayer->addChild(labelRates);


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
    width  = a_width;
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
			movingRight = 1;
		}
		else if (a_key == GLFW_KEY_A || a_key == GLFW_KEY_LEFT)
		{
			movingLeft = -1;
		}
		else if (a_key == GLFW_KEY_W || a_key == GLFW_KEY_UP)
		{
			movingForwards = -1;
		}
		else if (a_key == GLFW_KEY_S || a_key == GLFW_KEY_DOWN)
		{
			movingBackwards = 1;
		}
		else if (a_key == GLFW_KEY_SPACE)
		{
			hint = true;
		}
	}
	else if (a_action == GLFW_RELEASE)
	{
		if (a_key == GLFW_KEY_D || a_key == GLFW_KEY_RIGHT)
		{
			movingRight = 0;
		}
		else if (a_key == GLFW_KEY_A || a_key == GLFW_KEY_LEFT)
		{
			movingLeft = 0;
		}
		else if (a_key == GLFW_KEY_W || a_key == GLFW_KEY_UP)
		{
			movingForwards = 0;
		}
		else if (a_key == GLFW_KEY_S || a_key == GLFW_KEY_DOWN)
		{
			movingBackwards = 0;
		}
		else if (a_key == GLFW_KEY_SPACE)
		{
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
	player->translate(cVector3d(movingForwards+movingBackwards, movingRight+movingLeft, 0), delta_t);
	camera->set(player->body->getLocalPos() + cVector3d(0.05, 0.0, 0.05),    // camera position (eye)
		player->body->getLocalPos(),    // look at position (target)
		cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector
	//camera->attachAudioDevice(level->audioDevice);
	//level->audioDevice->setListenerPos(cVector3d(50.0, 0.0, 0.0));
	tool->setLocalPos(player->body->getLocalPos() + player->armDisplacement);
	
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
	// simulation in now running
	simulationRunning = true;
	simulationFinished = false;

	int curRoom = 0;

	std::string sorc[5];
	sorc[0] = "resources/music/april.wav";
	sorc[1] = "resources/music/april1.wav";
	sorc[2] = "resources/music/april2.wav";
	sorc[3] = "resources/music/april3.wav";
	sorc[4] = "resources/music/april4.wav";

	cAudioBuffer* buff[5];
	cAudioSource* source[5];

	for (int i = 0; i < 5; i++)
	{
		buff[i] = new cAudioBuffer();


		bool loadStatus;
		loadStatus = buff[i]->loadFromFile(sorc[i]);

		// check for errors
		if (!loadStatus)
		{
			cout << "Error - Sound file failed to load or initialize correctly." << endl;
			//close();
			//return (-1);
		}

		// create audio source
		source[i] = new cAudioSource();

		// assign audio buffer to audio source
		source[i]->setAudioBuffer(buff[i]);

		// set volume
		source[i]->setGain(0.0);

		// set speed at which the audio file is played. we will modulate this with the record speed.
		source[i]->setPitch(1.0);

		// loop audio play
		source[i]->setLoop(true);

		source[i]->setPosTime(60.f);

		source[i]->setSourcePos(level->rooms[12]->position);

		// start playing
		source[i]->play();
	}

	// main haptic simulation loop
	while (simulationRunning)
	{
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
		level->audioDevice->setListenerPos(tool->getLocalPos());


		/////////////////////////////////////////////////////////////////////
		// COMPUTE FORCES
		/////////////////////////////////////////////////////////////////////

		tool->computeInteractionForces();
		if (!hint)
		{
			level->audioDevice->setListenerPos(tool->getLocalPos() + tool->getDeviceLocalPos());
			double lvl = 0.1 - ((tool->getLocalPos() + tool->getDeviceLocalPos()) - level->rooms[12]->position).length();
			if (lvl < 0) lvl = 0.f;
			//cout << lvl << endl;
			//level->rooms[12]->audioSource->setGain(lvl);
			//for (int i = 0; i < 5; i++)
			//{
			//	source[i]->setGain(lvl);
			//}

		}
		else
		{
			level->audioDevice->setListenerPos(level->rooms[12]->audioSource->getSourcePos());
			//level->rooms[12]->audioSource->setGain(0.1);
			//for (int i = 0; i < 5; i++)
			//{
			//	source[i]->setGain(0.1);
			//}
		}

		int r = player->getRoom(level);
		//cout << r << endl;
		if (curRoom != r)
		{
			cout << "change" << endl;
			curRoom = r;

			for (int i = 0; i < 5; i++)
			{
				source[i]->setGain(0.0f);
			}
			switch (r)
			{
			case 0: source[4]->setGain(0.1f); break;
			case 1: source[3]->setGain(0.1f); break;
			case 2: source[3]->setGain(0.1f); break;
			case 3: source[4]->setGain(0.1f); break;
			case 4: source[3]->setGain(0.1f); break;
			case 5: source[3]->setGain(0.1f); break;
			case 6: source[2]->setGain(0.1f); break;
			case 7: source[2]->setGain(0.1f); break;
			case 8: source[1]->setGain(0.1f); break;
			case 9: source[1]->setGain(0.1f); break;
			case 10: source[2]->setGain(0.1f); break;
			case 11: source[1]->setGain(0.1f); break;
			case 12: source[0]->setGain(0.1f); break;
			case 13: source[0]->setGain(0.1f); break;
			case 14: source[0]->setGain(0.1f); break;
			case 15: source[0]->setGain(0.1f); break;
			}
		}

		


		cVector3d force(0, 0, 0);
		cVector3d torque(0, 0, 0);
		double gripperForce = 0.0;


		/////////////////////////////////////////////////////////////////////
		// APPLY FORCES
		/////////////////////////////////////////////////////////////////////


		tool->applyToDevice();

		// signal frequency counter
		freqCounterHaptics.signal(1);
	}

	// exit haptics thread
	simulationFinished = true;
}
