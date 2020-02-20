#pragma once
#include <windows.h>									// Header File For The Windows Library
#include "./include/fmod_studio/fmod.hpp"
#include "./include/fmod_studio/fmod_errors.h"
#include "./include/glm/gtc/type_ptr.hpp"
#include "Camera.h"



class CAudio
{
public:
	CAudio();
	~CAudio();
	bool Initialise();
	bool LoadEventSound(char *filename);
	bool PlayEventSound();
	bool LoadMusicStream(char *filename);
	bool PlayMusicStream();
	void ToggleMusicFilter();
	void IncreaseMusicVolume();
	void DecreaseMusicVolume();
	void Update(CCamera *cam);

private:
		

	void FmodErrorCheck(FMOD_RESULT result);
	void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec);


	FMOD_RESULT result;
	FMOD::System *m_FmodSystem;	// the global variable for talking to FMOD

	FMOD::Sound *m_eventSound;
	FMOD::Channel *m_eventChannel;
	
	FMOD::Sound *m_music;
	FMOD::DSP *m_musicFilter;
	bool m_musicFilterActive;
	FMOD::Channel *m_musicChannel;
	FMOD::DSP *m_musicDSPHead;
	FMOD::DSP *m_musicDSPHeadInput;
	float m_musicVolume = 0.2f;

	FMOD_VECTOR camPos;
	FMOD_VECTOR eventPos;
	FMOD_VECTOR eventVel;

};
