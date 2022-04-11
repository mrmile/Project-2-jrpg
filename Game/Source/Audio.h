#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Module.h"
#include "Point.h"
#include "ModulePlayer.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

enum Music
{
	OFF = 0,
	FINAL_BATTLE,
	TRUE_FINAL_BATTLE,
	REGULAR_BATTLE,
	SHIP_DECK,
	CAVE,
	BASE,
	MAIN_MAP,
	MOTEL_ZONE,
	RADIO_ITEM_MUSIC,
	RADIO_MESSAGE,
	COMPUTER_MESSAGE,
	LOGO_INTRO,
	TITLE_THEME,
	MENU,
};

struct _Mix_Music;
struct Mix_Chunk;

class Audio : public Module
{
public:

	Audio(bool start_enabled = false);

	// Destructor
	virtual ~Audio();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Play a music file
	bool PlayMusic(const char* path, float fadeInTime = DEFAULT_MUSIC_FADE_TIME, float fadeOutTime = DEFAULT_MUSIC_FADE_TIME);

	// Load a WAV in memory
	unsigned int LoadFx(const char* path);

	// Play a previously loaded WAV
	bool PlayFxSpatially(unsigned int fx, iPoint soundGeneratorPosition, int repeat = 0);

	bool PlayFx(unsigned int fx, int repeat = 0);

	// Change Music track
	bool ChangeMusic(int Id, float fadeInTime = DEFAULT_MUSIC_FADE_TIME, float fadeOutTime = DEFAULT_MUSIC_FADE_TIME);

	// Play Music track spatially (diegetic)
	bool PlayMusicSpatially(iPoint musicGeneratorPosition);

private:

	_Mix_Music* music;
	List<Mix_Chunk *>	fx;

public:

	int SliderLevelFX = 90;
	int SliderLevelMusic = 90;

	bool playMusicSpatially = false;

};

#endif // __AUDIO_H__