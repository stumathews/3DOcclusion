#include "Audio.h"

#include "OpenAssetImportMesh.h"

#pragma comment(lib, "lib/fmod_vc.lib")

CAudio::CAudio()
{}

CAudio::~CAudio()
{}

bool CAudio::Initialise()
{
	// Create an FMOD system
	result = FMOD::System_Create(&m_FmodSystem);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	// Initialise the system
	result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	// Set 3D settings - doppler effect, distance factor and rolloff scale
	result = m_FmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;
	
	// Place/draw the wall at the same location of the tomb wolf:

	glm::vec3 wolf_position(0.0f, 0.0f, -50.0f);
	
	const auto wall_start_x = 2.0f;
	const auto wall_end_x = -3.0f;
	const auto wall_height = 300.0f;
	const auto z = -10.00f;
	
	glm::vec3 v1(wall_start_x, 0.0f, z); // bottom left
	glm::vec3 v2(wall_start_x, wall_height, z); // top left 
	glm::vec3 v3(wall_end_x, wall_height, z); // bottom right
	glm::vec3 v4(wall_end_x, 0.0f, z); //bottom left

	FMOD::Geometry* geometry;
	FMOD_VECTOR wall[4];
	FMOD_VECTOR geometry_position;

	// The position of the objects in the geometry will be relative to wolf's position
	ToFMODVector(wolf_position, &geometry_position);
	
	m_FmodSystem->createGeometry(1, 4, &geometry);

	/* Add objects to this geometry group: 1 wall */

	auto poly_index = 0;	

	// set each 
	ToFMODVector(v1, &wall[0]);
	ToFMODVector(v2, &wall[1]);
	ToFMODVector(v3, &wall[2]);
	ToFMODVector(v4, &wall[3]);
	
	geometry->addPolygon(100.0f, 0.0f, FALSE, 4, wall, &poly_index);		
	// Set the position of the geometry
	geometry->setPosition(&geometry_position);
	geometry->setActive(TRUE);
	return true;
	
}

// Load an event sound
bool CAudio::LoadEventSound(char *filename)
{
	result = m_FmodSystem->createSound(filename, FMOD_LOOP_OFF, 0, &m_eventSound);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	return true;
}

// Play an event sound
bool CAudio::PlayEventSound()
{
	result = m_FmodSystem->playSound(m_eventSound, NULL, false, &m_eventChannel);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;
	// play through 3D channel
	m_eventChannel->setMode(FMOD_3D);
	m_eventChannel->setVolume(100);
	// set the position to be the horse's position
	result = m_eventChannel->set3DAttributes(0, 0, 0);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;
	return true;
}


// Load a music stream
bool CAudio::LoadMusicStream(char *filename)
{
	result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	// create a low-pass filter DSP object
	result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_musicFilter);

	if (result != FMOD_OK)
		return false;

	// you can start the DSP in an inactive state
	m_musicFilter->setActive(false);

	return true;
	

}

// Play a music stream
bool CAudio::PlayMusicStream()
{
	result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	// Set the volume lower
	result = m_musicChannel->setVolume(m_musicVolume);

	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	// connecting the music filter to the music stream
	// Get the DSP head and it's input
	m_musicChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &m_musicDSPHead);
	m_musicDSPHead->getInput(0, &m_musicDSPHeadInput, NULL);
	// Disconnect them
	m_musicDSPHead->disconnectFrom(m_musicDSPHeadInput);
	// Add input to the music head from the filter
	result = m_musicDSPHead->addInput(m_musicFilter);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	// Add input to the filter head music DSP head input
	result = m_musicFilter->addInput(m_musicDSPHeadInput);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	// set the DSP object to be active
	m_musicFilter->setActive(true);
	// initially set the cutoff to a high value
	m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
	// this state is used for toggling
	m_musicFilterActive = false;

	return true;
}

// Check for error
void CAudio::FmodErrorCheck(FMOD_RESULT result)
{						
	if (result != FMOD_OK) {
		const char *errorString = FMOD_ErrorString(result);
		// MessageBox(NULL, errorString, "FMOD Error", MB_OK);
		// Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
	}
}

void CAudio::Update(CCamera *camera)
{
	// 5) update the listener's position with the camera position
	ToFMODVector(camera->GetPosition(), &camPos);
	result = m_FmodSystem->set3DListenerAttributes(0, &camPos, NULL, NULL, NULL);
	FmodErrorCheck(result);
	m_FmodSystem->update();
}

void CAudio::ToggleMusicFilter()
{
	// called externally from Game::ProcessEvents
	// toggle the effect on/off
	m_musicFilterActive = !m_musicFilterActive;
	if (m_musicFilterActive) {
		// set the parameter to a low value
		m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 700);
	}
	else {
		// set the parameter to a high value
		// you could also use m_musicFilter->setBypass(true) instead...
		m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
	}
}

void CAudio::IncreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// increment the volume
	m_musicVolume += 0.05f;
	if (m_musicVolume > 1)
		m_musicVolume = 1.0f;
	m_musicChannel->setVolume(m_musicVolume);
}

void CAudio::DecreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// deccrement the volume
	m_musicVolume -= 0.05f;
	if (m_musicVolume < 0)
		m_musicVolume = 0.0f;
	m_musicChannel->setVolume(m_musicVolume);
}

// implementation changing open gl vectors to fmod vectors
void CAudio::ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec)
{
	fmodVec->x = glVec3.x;
	fmodVec->y = glVec3.y;
	fmodVec->z = glVec3.z;
}