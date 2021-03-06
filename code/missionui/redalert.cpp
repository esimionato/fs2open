/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#define REDALERT_INTERNAL
#include "missionui/redalert.h"
#include "model/model.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionweaponchoice.h"
#include "io/key.h"
#include "graphics/font.h"
#include "mission/missionbriefcommon.h"
#include "io/timer.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "globalincs/linklist.h"
#include "hud/hudwingmanstatus.h"
#include "sound/audiostr.h"
#include "freespace2/freespace.h"
#include "globalincs/alphacolors.h"
#include "sound/fsspeech.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "cfile/cfile.h"
#include "io/mouse.h"
#include "ai/aigoals.h"

#include <stdexcept>


/////////////////////////////////////////////////////////////////////////////
// Red Alert Mission-Level
/////////////////////////////////////////////////////////////////////////////

static int Red_alert_new_mission_timestamp;		// timestamp used to give user a little warning for red alerts
//static int Red_alert_num_slots_used = 0;
static int Red_alert_voice_started;

SCP_vector<red_alert_ship_status> Red_alert_wingman_status;
SCP_string Red_alert_precursor_mission;

/////////////////////////////////////////////////////////////////////////////
// Red Alert Interface
/////////////////////////////////////////////////////////////////////////////

char *Red_alert_fname[GR_NUM_RESOLUTIONS] = {
	"RedAlert",
	"2_RedAlert"
};

char *Red_alert_mask[GR_NUM_RESOLUTIONS] = {
	"RedAlert-m",
	"2_RedAlert-m"
};

// font to use for "incoming transmission"
int Ra_flash_font[GR_NUM_RESOLUTIONS] = {
	FONT2,
	FONT2
};

int Ra_flash_y[GR_NUM_RESOLUTIONS] = {
	140,
	200
};

/*
static int Ra_flash_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		61, 108			// GR_640
	},
	{
		61, 108			// GR_1024
	}
};
*/

#define NUM_BUTTONS						2

#define RA_REPLAY_MISSION				0
#define RA_CONTINUE						1

static ui_button_info Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{	// GR_640
		ui_button_info("RAB_00",	2,		445,	-1,	-1, 0),
		ui_button_info("RAB_01",	575,	432,	-1,	-1, 1),
	},	
	{	// GR_1024
		ui_button_info("2_RAB_00",	4,		712,	-1,	-1, 0),
		ui_button_info("2_RAB_01",	920,	691,	-1,	-1, 1),
	}
};

#define RED_ALERT_NUM_TEXT		3
UI_XSTR Red_alert_text[GR_NUM_RESOLUTIONS][RED_ALERT_NUM_TEXT] = {
	{ // GR_640
		{ "Replay",		1405,	46,	451,	UI_XSTR_COLOR_PINK,	-1, &Buttons[0][RA_REPLAY_MISSION].button },
		{ "Previous Mission",	1452,	46,	462,	UI_XSTR_COLOR_PINK,	-1, &Buttons[0][RA_REPLAY_MISSION].button },
		{ "Continue",	1069,	564,	413,	UI_XSTR_COLOR_PINK,	-1, &Buttons[0][RA_CONTINUE].button },
	},
	{ // GR_1024
		{ "Replay",		1405,	75,	722,	UI_XSTR_COLOR_PINK,	-1, &Buttons[1][RA_REPLAY_MISSION].button },
		{ "Previous Mission",	1452,	75,	733,	UI_XSTR_COLOR_PINK,	-1, &Buttons[1][RA_REPLAY_MISSION].button },
		{ "Continue",	1069,	902,	661,	UI_XSTR_COLOR_PINK,	-1, &Buttons[1][RA_CONTINUE].button },
	}
};

// indicies for coordinates
#define RA_X_COORD 0
#define RA_Y_COORD 1
#define RA_W_COORD 2
#define RA_H_COORD 3


static int Text_delay;

int Ra_brief_text_wnd_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		14, 151, 522, 289
	},
	{
		52, 241, 785, 463
	}
};

static UI_WINDOW Ui_window;
// static hud_anim Flash_anim;
static int Background_bitmap;
static int Red_alert_inited = 0;

static int Red_alert_voice;

// open and pre-load the stream buffers for the different voice streams
void red_alert_voice_load()
{
	Assert( Briefing != NULL );
	if ( strnicmp(Briefing->stages[0].voice, NOX("none"), 4) && (Briefing->stages[0].voice[0] != '\0') ) {
		Red_alert_voice = audiostream_open( Briefing->stages[0].voice, ASF_VOICE );
	}
}

// close all the briefing voice streams
void red_alert_voice_unload()
{
	if ( Red_alert_voice != -1 ) {
		audiostream_close_file(Red_alert_voice, 0);
		Red_alert_voice = -1;
	}
}

// start playback of the red alert voice
void red_alert_voice_play()
{
	if ( !Briefing_voice_enabled ) {
		return;
	}

	if ( Red_alert_voice < 0 ) {
		// play simulated speech?
		if (fsspeech_play_from(FSSPEECH_FROM_BRIEFING)) {
			if (fsspeech_playing()) {
				return;
			}

			fsspeech_play(FSSPEECH_FROM_BRIEFING, Briefing->stages[0].text.c_str());
			Red_alert_voice_started = 1;
		}
	} else {
		if (audiostream_is_playing(Red_alert_voice)) {
			return;
		}

		audiostream_play(Red_alert_voice, Master_voice_volume, 0);
		Red_alert_voice_started = 1;
	}
}

// stop playback of the red alert voice
void red_alert_voice_stop()
{
	if ( !Red_alert_voice_started )
		return;

	if (Red_alert_voice < 0) {
		fsspeech_stop();
	} else {
		audiostream_stop(Red_alert_voice, 1, 0);	// stream is automatically rewound
	}
}

// pausing and unpausing of red alert voice
void red_alert_voice_pause()
{
	if ( !Red_alert_voice_started )
		return;

	if (Red_alert_voice < 0) {
		fsspeech_pause(true);
	} else {
		audiostream_pause(Red_alert_voice);
	}
}

void red_alert_voice_unpause()
{
	if ( !Red_alert_voice_started )
		return;

	if (Red_alert_voice < 0) {
		fsspeech_pause(false);
	} else {
		audiostream_unpause(Red_alert_voice);
	}
}

// a button was pressed, deal with it
void red_alert_button_pressed(int n)
{
	switch (n) {
	case RA_CONTINUE:		
		// warp the mouse cursor the the middle of the screen for those who control with a mouse
		mouse_set_pos( gr_screen.max_w/2, gr_screen.max_h/2 );

		if(Game_mode & GM_MULTIPLAYER){	
			// process the initial orders now (moved from post_process_mission()in missionparse) 
			mission_parse_fixup_players();
			ai_post_process_mission();
		}

		gameseq_post_event(GS_EVENT_ENTER_GAME);
		break;

	case RA_REPLAY_MISSION:
		if ( Game_mode & GM_CAMPAIGN_MODE ) {
			// TODO: make call to campaign code to set correct mission for loading
			// mission_campaign_play_previous_mission(Red_alert_precursor_mission);
			if ( !mission_campaign_previous_mission() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			gameseq_post_event(GS_EVENT_START_GAME);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;
	}
}

// blit "incoming transmission"
#define RA_FLASH_CYCLE			0.25f
float Ra_flash_time = 0.0f;
int Ra_flash_up = 0;
void red_alert_blit_title()
{
	char *str = XSTR("Incoming Transmission", 1406);
	int w, h;

	// get the string size	
	gr_set_font(Ra_flash_font[gr_screen.res]);
	gr_get_string_size(&w, &h, str);

	// set alpha color
	color flash_color;
	if(Ra_flash_up){
		gr_init_alphacolor(&flash_color, (int)(255.0f * (Ra_flash_time / RA_FLASH_CYCLE)), 0, 0, 255);
	} else {
		gr_init_alphacolor(&flash_color, (int)(255.0f * (1.0f - (Ra_flash_time / RA_FLASH_CYCLE))), 0, 0, 255);
	}

	// draw
	gr_set_color_fast(&flash_color);
	gr_string(Ra_brief_text_wnd_coords[gr_screen.res][0] + ((Ra_brief_text_wnd_coords[gr_screen.res][2] - w) / 2), Ra_flash_y[gr_screen.res] - h - 5, str);
	gr_set_color_fast(&Color_normal);	

	// increment flash time
	Ra_flash_time += flFrametime;
	if(Ra_flash_time >= RA_FLASH_CYCLE){
		Ra_flash_time = 0.0f;
		Ra_flash_up = !Ra_flash_up;
	}

	// back to the original font
	gr_set_font(FONT1);
}

// Called once when red alert interface is started
void red_alert_init()
{
	int i;
	ui_button_info *b;

	if ( Red_alert_inited ) {
		return;
	}

	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Red_alert_mask[gr_screen.res]);

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// all text
	for(i=0; i<RED_ALERT_NUM_TEXT; i++){
		Ui_window.add_XSTR(&Red_alert_text[gr_screen.res][i]);
	}

	// set up red alert hotkeys
	Buttons[gr_screen.res][RA_CONTINUE].button.set_hotkey(KEY_CTRLED | KEY_ENTER);

	// load in background image and flashing red alert animation
	Background_bitmap = bm_load(Red_alert_fname[gr_screen.res]);
	
	// hud_anim_init(&Flash_anim, Ra_flash_coords[gr_screen.res][RA_X_COORD], Ra_flash_coords[gr_screen.res][RA_Y_COORD], NOX("AlertFlash"));
	// hud_anim_load(&Flash_anim);

	Red_alert_voice = -1;

	if ( !Briefing ) {
		Briefing = &Briefings[0];			
	}

	if ( Briefing->num_stages > 0 ) {
		brief_color_text_init(Briefing->stages[0].text.c_str(), Ra_brief_text_wnd_coords[gr_screen.res][RA_W_COORD], 0);
	}

	red_alert_voice_load();

	// we have to reset/setup the shipselect and weaponselect pointers before moving on
	ship_select_common_init();
	weapon_select_common_init();

	Text_delay = timestamp(200);

	Red_alert_voice_started = 0;
	Red_alert_inited = 1;
}

// Called once when the red alert interface is exited
void red_alert_close()
{
	if (Red_alert_inited) {

		red_alert_voice_stop();
		red_alert_voice_unload();

		weapon_select_close_team();

		if (Background_bitmap >= 0) {
			bm_release(Background_bitmap);
		}
		
		Ui_window.destroy();
		// bm_unload(&Flash_anim);
		common_free_interface_palette();		// restore game palette
		game_flush();
	}

	Red_alert_inited = 0;

	fsspeech_stop();
}

// called once per frame when game state is GS_STATE_RED_ALERT
void red_alert_do_frame(float frametime)
{
	int i, k;	

	// ensure that the red alert interface has been initialized
	if (!Red_alert_inited) {
		Int3();
		return;
	}

	// commit if skipping briefing, but not in multi - Goober5000
	if (!(Game_mode & GM_MULTIPLAYER)) {
		if (The_mission.flags & MISSION_FLAG_NO_BRIEFING)
		{
			commit_pressed();
			return;
		}
	}

	k = Ui_window.process() & ~KEY_DEBUGGED;
	switch (k) {
		case KEY_ESC:
//			gameseq_post_event(GS_EVENT_ENTER_GAME);
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;
	}	// end switch

	for (i=0; i<NUM_BUTTONS; i++){
		if (Buttons[gr_screen.res][i].button.pressed()){
			red_alert_button_pressed(i);
		}
	}

	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);
	} 

	Ui_window.draw();
	// hud_anim_render(&Flash_anim, frametime);

	gr_set_font(FONT1);

	if ( timestamp_elapsed(Text_delay) ) {
		int finished_wipe = 0;
		if ( Briefing->num_stages > 0 ) {
			finished_wipe = brief_render_text(0, Ra_brief_text_wnd_coords[gr_screen.res][RA_X_COORD], Ra_brief_text_wnd_coords[gr_screen.res][RA_Y_COORD], Ra_brief_text_wnd_coords[gr_screen.res][RA_H_COORD], frametime, 0);
		}

		if (finished_wipe) {
			red_alert_voice_play();
		}
	}

	// blit incoming transmission
	red_alert_blit_title();

	gr_flip();
}

// set the red alert status for the current mission
void red_alert_invalidate_timestamp()
{
	Red_alert_new_mission_timestamp = timestamp(-1);		// make invalid
}

// Store a ships weapons into a wingman status structure
void red_alert_store_weapons(red_alert_ship_status *ras, ship_weapon *swp)
{
	int i;
	weapon_info *wip;
	wep_t weapons;

	// Make sure there isn't any data from the previous ship.
	ras->primary_weapons.clear();
	ras->secondary_weapons.clear();

	if (swp == NULL) {
		return;
	}

	// edited to accommodate ballistics - Goober5000
	for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
		weapons.index = swp->primary_bank_weapons[i];

		if (weapons.index < 0) {
			continue;
		}

		wip = &Weapon_info[weapons.index];

		if (wip->wi_flags2 & WIF2_BALLISTIC) {
			// to avoid triggering the below condition: this way, minimum ammo will be 2...
			// since the red-alert representation of a conventional primary is 0 -> not used,
			// 1 -> used, I added the representation 2 and above -> ballistic primary
			weapons.count = swp->primary_bank_ammo[i] + 2;
		} else {
			weapons.count = 1;
		}

		ras->primary_weapons.push_back( weapons );
	}

	for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		weapons.index = swp->secondary_bank_weapons[i];

		if (weapons.index < 0) {
			continue;
		}

		weapons.count = swp->secondary_bank_ammo[i];

		ras->secondary_weapons.push_back( weapons );
	}
}

// Take the weapons stored in a wingman_status struct, and bash them into the ship weapons struct
void red_alert_bash_weapons(red_alert_ship_status *ras, ship_weapon *swp)
{
	int i, list_size = 0;

	// restore from ship_exited
	if ( (ras->ship_class == RED_ALERT_DESTROYED_SHIP_CLASS) || (ras->ship_class == RED_ALERT_PLAYER_DEL_SHIP_CLASS) ) {
		return;
	}

	// modified to accommodate ballistics - Goober5000
	list_size = (int)ras->primary_weapons.size();
	CLAMP(list_size, 0, MAX_SHIP_PRIMARY_BANKS);
	for (i = 0; i < list_size; i++) {
		Assert( ras->primary_weapons[i].index >= 0 );

		swp->primary_bank_weapons[i] = ras->primary_weapons[i].index;
		swp->primary_bank_ammo[i] = ras->primary_weapons[i].count;
	}
	swp->num_primary_banks = list_size;


	list_size = (int)ras->secondary_weapons.size();
	CLAMP(list_size, 0, MAX_SHIP_SECONDARY_BANKS);
	for (i = 0; i < list_size; i++) {
		Assert( ras->secondary_weapons[i].index >= 0 );

		swp->secondary_bank_weapons[i] = ras->secondary_weapons[i].index;
		swp->secondary_bank_ammo[i] = ras->secondary_weapons[i].count;
	}
	swp->num_secondary_banks = list_size;
}

void red_alert_bash_subsys_status(red_alert_ship_status *ras, ship *shipp)
{
	ship_subsys *ss;
	int i, count = 0;
	int list_size;

	// restore from ship_exited
	if ( (ras->ship_class == RED_ALERT_DESTROYED_SHIP_CLASS) || (ras->ship_class == RED_ALERT_PLAYER_DEL_SHIP_CLASS) ) {
		return;
	}

	ss = GET_FIRST(&shipp->subsys_list);
	while ( ss != END_OF_LIST( &shipp->subsys_list ) ) {
		// using at() here for the bounds check, although out-of-bounds should
		// probably never happen here
		try {
			ss->current_hits = ras->subsys_current_hits.at(count);
		} catch (std::out_of_range range) {
			break;
		}

		if (ss->current_hits <= 0) {
			ss->submodel_info_1.blown_off = 1;
		}

		ss = GET_NEXT( ss );
		count++;
	}

	list_size = (int)ras->subsys_aggregate_current_hits.size();
	CLAMP(list_size, 0, SUBSYSTEM_MAX);
	for (i = 0; i < list_size; i++) {
		shipp->subsys_info[i].aggregate_current_hits = ras->subsys_aggregate_current_hits[i];
	}
}


void red_alert_store_subsys_status(red_alert_ship_status *ras, ship *shipp)
{
	ship_subsys *ss;
	int i;
	
	// Make sure there isn't any data from the previous ship.
	ras->subsys_current_hits.clear();
	ras->subsys_aggregate_current_hits.clear();
	
	if (shipp == NULL) {
		return;
	}

	ss = GET_FIRST(&shipp->subsys_list);
	while ( ss != END_OF_LIST( &shipp->subsys_list ) ) {
		ras->subsys_current_hits.push_back( ss->current_hits );

		ss = GET_NEXT( ss );
	}

	for (i = 0; i < SUBSYSTEM_MAX; i++)
		// Pyro3d - Fixes AP8 Based RA crashes
		ras->subsys_aggregate_current_hits.push_back(shipp->subsys_info[i].aggregate_current_hits);
}


/*
 * Record the current state of the players wingman & ships with the "red-alert-carry" flag
 * Wingmen without the red-alert-carry flag are only stored if they survive
 * dead wingmen must still be handled in red_alert_bash_wingman_status
 */
void red_alert_store_wingman_status()
{
	ship				*shipp;
	red_alert_ship_status	ras;
	ship_obj			*so;
	object			*ship_objp;

	// store the mission filename for the red alert precursor mission
	Red_alert_precursor_mission = Game_current_mission_filename;

	// Pyro3d - Clear list of stored red alert ships 
	// Probably not the best solution, but it prevents an assertion in change_ship_type()
	Red_alert_wingman_status.clear();

	// store status for all existing ships
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship_objp = &Objects[so->objnum];
		Assert(ship_objp->type == OBJ_SHIP);
		shipp = &Ships[ship_objp->instance];

		if ( shipp->flags & SF_DYING ) {
			continue;
		}

		if ( !(shipp->flags & SF_FROM_PLAYER_WING) && !(shipp->flags & SF_RED_ALERT_STORE_STATUS) ) {
			continue;
		}

		ras.name = shipp->ship_name;
		ras.hull = Objects[shipp->objnum].hull_strength;
		ras.ship_class = shipp->ship_info_index;
		red_alert_store_weapons(&ras, &shipp->weapons);
		red_alert_store_subsys_status(&ras, shipp);

		Red_alert_wingman_status.push_back( ras );
		// niffiwan: trying to track down red alert bug creating HUGE pilot files 
		Assert( (Red_alert_wingman_status.size() <= MAX_SHIPS) );
	}

	// store exited ships that did not die
	for (int idx=0; idx<(int)Ships_exited.size(); idx++) {
		if (Ships_exited[idx].flags & SEF_RED_ALERT_CARRY) {
			ras.name = Ships_exited[idx].ship_name;
			ras.hull = float(Ships_exited[idx].hull_strength);

			// if a ship has been destroyed or removed manually by the player, then mark it as such ...
			if ( Ships_exited[idx].flags & SEF_DESTROYED ) {
				ras.ship_class = RED_ALERT_DESTROYED_SHIP_CLASS;
			}
			else if (Ships_exited[idx].flags & SEF_PLAYER_DELETED) {
				ras.ship_class = RED_ALERT_PLAYER_DEL_SHIP_CLASS;
			}
			// ... otherwise we want to make sure and carry over the ship class
			else {
				Assert( Ships_exited[idx].ship_class >= 0 );
				ras.ship_class = Ships_exited[idx].ship_class;
			}

			red_alert_store_weapons(&ras, NULL);
			red_alert_store_subsys_status(&ras, NULL);

			Red_alert_wingman_status.push_back( ras );
			// niffiwan: trying to track down red alert bug creating HUGE pilot files 
			Assert( (Red_alert_wingman_status.size() <= MAX_SHIPS) );
		}
	}

	Assert( !Red_alert_wingman_status.empty() );
}

// Delete a ship in a red alert mission (since it must have died/departed in the previous mission)
void red_alert_delete_ship(ship *shipp, int ship_state)
{
	if ( (shipp->wing_status_wing_index >= 0) && (shipp->wing_status_wing_pos >= 0) ) {
		if (ship_state == RED_ALERT_DESTROYED_SHIP_CLASS) {
			hud_set_wingman_status_dead(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
		} else if (ship_state == RED_ALERT_PLAYER_DEL_SHIP_CLASS) {
			hud_set_wingman_status_none(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
		} else {
			Error(LOCATION, "Red Alert: asked to delete ship (%s) with invalid ship state (%d)", shipp->ship_name, ship_state);
		}
	}

	ship_add_exited_ship( shipp, SEF_PLAYER_DELETED );
	obj_delete(shipp->objnum);
	if ( shipp->wingnum >= 0 ) {
		ship_wing_cleanup( shipp-Ships, &Wings[shipp->wingnum] );
	}
}

/*
 * Take the red alert status information, and adjust the red alert ships accordingly
 * "red alert ships" are wingmen and any ship with the red-alert-carry flag
 * Wingmen without red alert data still need to be handled / removed
 */
void red_alert_bash_wingman_status()
{
	int				i;
	ship				*shipp;
	red_alert_ship_status	*ras;
	ship_obj			*so;
	object			*ship_objp;

	if ( !(Game_mode & GM_CAMPAIGN_MODE) ) {
		return;
	}

	if ( Red_alert_wingman_status.empty() ) {
		return;
	}

	// go through all ships in the game, and see if there is red alert status data for any

	int remove_list[MAX_SHIPS];
	int remove_state[MAX_SHIPS];
	int remove_count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship_objp = &Objects[so->objnum];
		Assert(ship_objp->type == OBJ_SHIP);
		shipp = &Ships[ship_objp->instance];

		if ( !(shipp->flags & SF_FROM_PLAYER_WING) && !(shipp->flags & SF_RED_ALERT_STORE_STATUS) ) {
			continue;
		}

		int found_match = 0;
		int ship_state = 0;

		for ( i = 0; i < (int)Red_alert_wingman_status.size(); i++ ) {
			ras = &Red_alert_wingman_status[i];

			// we only want to restore ships which haven't been destroyed, or were removed by the player
			if ( !stricmp(ras->name.c_str(), shipp->ship_name) && (ras->ship_class != RED_ALERT_DESTROYED_SHIP_CLASS) && (ras->ship_class != RED_ALERT_PLAYER_DEL_SHIP_CLASS) ) {
				found_match = 1;

				// if necessary, restore correct ship class
				if ( ras->ship_class != shipp->ship_info_index ) {
					if (ras->ship_class < MAX_SHIP_CLASSES && ras->ship_class > -1) {
						change_ship_type(SHIP_INDEX(shipp), ras->ship_class);
					} else {
						mprintf(("Invalid ship class specified in red alert data for ship %s. Using mission defaults.\n", shipp->ship_name));
					}
				}
				// restore hull and weapons
				if (ras->hull >= 0.0f && ras->hull <= ship_objp->hull_strength) {
					ship_objp->hull_strength = ras->hull;
				} else {
					mprintf(("Invalid health in red alert data for ship %s. Using mission defaults.\n", shipp->ship_name));
				}
				red_alert_bash_weapons(ras, &shipp->weapons);
				red_alert_bash_subsys_status(ras, shipp);

			} else if ( !stricmp(ras->name.c_str(), shipp->ship_name) && ( (ras->ship_class == RED_ALERT_DESTROYED_SHIP_CLASS) || (ras->ship_class == RED_ALERT_PLAYER_DEL_SHIP_CLASS) ) ) {
				ship_state = ras->ship_class;
				continue;
			}
		}

		if ( !found_match ) {
			remove_state[remove_count] = ship_state;
			remove_list[remove_count++] = SHIP_INDEX(shipp);
		}
	}

	// remove ships
	for ( i = 0; i < remove_count; i++ ) {
		// remove ship
		red_alert_delete_ship(&Ships[remove_list[i]], remove_state[i]);
	}
}

// return !0 if this is a red alert mission, otherwise return 0
int red_alert_mission()
{
	return (The_mission.flags & MISSION_FLAG_RED_ALERT);
}

// called from sexpression code to start a red alert mission
void red_alert_start_mission()
{
	// if we are not in campaign mode, go to debriefing
//	if ( !(Game_mode & GM_CAMPAIGN_MODE) ) {
//		gameseq_post_event( GS_EVENT_DEBRIEF );	// proceed to debriefing
//		return;
//	}

	// check player health here.  
	// if we're dead (or about to die), don't start red alert mission.
	if (Player_obj->type == OBJ_SHIP) {
		if (Player_obj->hull_strength > 0) {
			// make sure we don't die
			Player_ship->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;

			// do normal red alert stuff
			Red_alert_new_mission_timestamp = timestamp(RED_ALERT_WARN_TIME);

			// throw down a sound here to make the warning seem ultra-important
			// gamesnd_play_iface(SND_USER_SELECT);
			snd_play(&(Snds[SND_DIRECTIVE_COMPLETE]));
		}
	}
}

// called from HUD code to see if we're red-alerting
int red_alert_in_progress()
{
	// it is specifically a question of whether the timestamp is running
	return timestamp_valid(Red_alert_new_mission_timestamp);
}

// called from the game loop to check if we should actually do the red-alert
void red_alert_maybe_move_to_next_mission()
{
	// if the timestamp is invalid, do nothing.
	if ( !timestamp_valid(Red_alert_new_mission_timestamp) )
		return;

	// return if the timestamp hasn't elapsed yet
	if ( !timestamp_elapsed(Red_alert_new_mission_timestamp) )
		return;

	// basic premise here is to stop the current mission, and then set the next mission in the campaign
	// which better be a red alert mission
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		red_alert_store_wingman_status();
		mission_goal_fail_incomplete();
		mission_campaign_store_goals_and_events_and_variables();
		scoring_level_close();
		mission_campaign_eval_next_mission();
		mission_campaign_mission_over();

		// CD CHECK
		gameseq_post_event(GS_EVENT_START_GAME);
	} else {
		gameseq_post_event(GS_EVENT_END_GAME);
	}
}
