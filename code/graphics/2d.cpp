/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/2d.cpp $
 * $Revision: 2.20 $
 * $Date: 2004-03-08 18:36:21 $
 * $Author: randomtiger $
 *
 * Main file for 2d primitives.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.19  2004/03/08 15:06:24  Kazan
 * Did, undo
 *
 * Revision 2.18  2004/03/07 23:07:20  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 2.17  2004/02/28 14:14:56  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.16  2004/02/16 11:47:32  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.15  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.14  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.13  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.12  2003/11/17 06:52:52  bobboau
 * got assert to work again
 *
 * Revision 2.11  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.10  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.9  2003/10/16 00:17:14  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
 * Revision 2.8  2003/10/10 03:59:40  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.7  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.6  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.5  2003/03/02 05:41:52  penguin
 * Added some #ifndef NO_SOFTWARE_RENDERING
 *  - penguin
 *
 * Revision 2.4  2002/11/18 21:32:11  phreak
 * added some ogl functionality - phreak
 *
 * Revision 2.3.2.9  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.3.2.8  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.3.2.7  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.3.2.6  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.3.2.5  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.3  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/30 14:29:15  unknownplayer
 *
 * Started work on DX8.1 implementation. Updated the project files to encompass
 * the new files. Disable the compiler tag to use old DX code (THERE IS NO
 * NEW CODE YET!)
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.5  2002/05/26 14:10:29  mharris
 * More testing
 *
 * Revision 1.4  2002/05/17 23:44:22  mharris
 * Removed Int3()'s from opengl calls
 *
 * Revision 1.3  2002/05/17 02:52:38  mharris
 * More porting hacks
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 25    8/20/99 2:09p Dave
 * PXO banner cycling.
 * 
 * 24    8/16/99 9:45a Jefff
 * changes to cursor management to allow a 2nd temporary cursor
 * 
 * 23    7/27/99 3:52p Dave
 * Make star drawing a bit more robust to help lame D3D cards.
 * 
 * 22    7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 21    7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 20    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 19    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 18    7/12/99 11:42a Jefff
 * Made rectangle drawing smarter in D3D. Made plines draw properly on Ati
 * Rage Pro.
 * 
 * 17    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 16    7/02/99 3:05p Anoop
 * Oops. Fixed g3_draw_2d_poly() so that it properly handles poly bitmap
 * and LFB bitmap calls.
 * 
 * 15    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 14    4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 13    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 12    1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 11    1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 10    1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 9     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 8     12/18/98 1:49a Dave
 * Fixed Fred initialization problem resulting from hi-res mode changes.
 * 
 * 7     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 6     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 5     11/30/98 5:31p Dave
 * Fixed up Fred support for software mode.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 122   6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 121   5/23/98 11:26a Hoffoss
 * Fixed bug where optimized code used EDX and my code trashed it.
 * 
 * 120   5/22/98 11:09p John
 * put in code to hopefull not crash cyrix cpus
 * 
 * 119   5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 118   5/20/98 12:59p Hoffoss
 * Changed temporary crossfade code to just pop the image again.
 * 
 * 117   5/16/98 1:18p John
 * Made softtware DirectDraw reset palette after Alt+TAB.
 * 
 * 116   5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 115   5/08/98 10:49a John
 * Made 'gr d' go into direct3d mode.
 * 
 * 114   5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 113   4/21/98 9:28a John
 * Added stub for cross-fade.
 * 
 * 112   4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 111   4/06/98 12:55p John
 * Upped the gamma for Fred.
 * 
 * 110   3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 109   3/24/98 3:58p John
 * Put in (hopefully) final gamma setting code.
 * 
 * 108   3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 107   2/25/98 2:37p John
 * Made afterburner shake 2x less noticable.  Made 'gr a' leave Glide mode
 * properly.  Made model_caching never work in hardware mode.
 * 
 * 106   2/23/98 2:27p John
 * Made int3,asserts, etc pass video through.
 * 
 * 105   2/20/98 3:13p John
 * Made popup save code print an error an exit out if 3d accelerated.
 * 
 * 104   2/19/98 6:13p John
 * Made Glide do texturing & zbuffering.
 * 
 * 103   1/26/98 5:12p John
 * Added in code for Pentium Pro specific optimizations. Speed up
 * zbuffered correct tmapper about 35%.   Speed up non-zbuffered scalers
 * by about 25%.
 * 
 * 102   1/14/98 11:39a Dave
 * Polished up a bunch of popup support items.
 * 
 * 101   1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 100   12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 99    12/04/97 12:09p John
 * Made glows use scaler instead of tmapper so they don't rotate.  Had to
 * add a zbuffered scaler.
 * 
 * 98    12/03/97 10:47a John
 * added functions to save/restore entire screens.
 * 
 * 97    12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 96    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 95    11/24/97 11:20a John
 * added new mode
 * 
 * 94    11/14/97 3:54p John
 * Added triple buffering.
 * 
 * 93    11/14/97 12:30p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 92    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 91    9/20/97 8:16a John
 * Made .clr files go into the Cache directory. Replaced cfopen(name,NULL)
 * to delete a file with cf_delete.
 * 
 * 90    9/09/97 11:01a Sandeep
 * fixed warning level 4 bugs
 * 
 * 89    9/07/97 10:01p Lawrance
 * add in support for animating mouse pointer
 * 
 * 88    9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 87    8/20/97 4:19p John
 * added delay to hopefully fix mike's problems when getting int3's in
 * fullscreen.
 * 
 * 86    7/16/97 3:07p John
 * 
 * 85    6/18/97 12:07p John
 * fixed some color bugs
 * 
 * 84    6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 83    6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 82    6/12/97 5:04p John
 * Initial rev of Glide support
 * 
 * 81    6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 80    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 79    6/06/97 5:03p John
 * fixed bug withalpha colors failing after gr_init
 * 
 * 78    6/06/97 4:53p John
 * made gr_init not bash current color
 * 
 * 77    6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 76    5/20/97 10:36a John
 * Fixed problem with user bitmaps and direct3d caching.
 * 
 * 75    5/16/97 9:11a John
 * fixed bug that made Ctrl+Break in fullscreen hang
 * 
 * 74    5/14/97 4:38p John
 * Fixed print_screen bug.
 * 
 * 73    5/14/97 2:10p John
 * added rudimentary support for windowed direct3d.
 * 
 * 72    5/14/97 10:53a John
 * fixed some discrepencies between d3d and software palette setting.
 * 
 * 71    5/13/97 4:39p John
 * Added console function to set graphics modes.
 * 
 * 70    5/13/97 12:39p John
 * Got fullscreen mode working.
 * 
 * 69    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 68    5/07/97 2:59p John
 * Initial rev of D3D texturing.
 * 
 * 67    5/01/97 3:32p John
 * oops... forced 2d to always be in 8bpp,
 * 
 * 66    5/01/97 3:23p John
 * first hooks for some direct 3d setup stuff.
 * 
 * 65    4/28/97 5:33p John
 * fixed a newly introduced bug with fullscreen toggle.
 * 
 * 64    4/28/97 4:46p John
 * 
 * 63    4/22/97 12:20p John
 * fixed more resource leaks
 * 
 * 62    4/22/97 10:33a John
 * fixed the 2d resource leaks that Alan found.
 * 
 * 61    3/31/97 9:45a Allender
 * start of new font stuff
 * 
 * 60    3/26/97 10:52a Lawrance
 * mouse always on in menus, disappears in gameplay after 1 second
 * 
 * 59    3/14/97 3:55p John
 * Made tiled tmapper not always be zbuffered.
 * 
 * 58    3/13/97 9:09a Allender
 * variable to allow trashing of the palette.  kind of temporary code
 * right now...checking needed for undefined references
 * 
 * 57    3/12/97 2:51p John
 * Added some test code for tmapper.  
 * 
 * 56    3/12/97 9:25a John
 * fixed a bug with zbuffering.  Reenabled it by default.
 * 
 * 55    3/11/97 5:13p John
 * made top up bitmaps work.
 * 
 * 54    2/26/97 11:59a Allender
 * comment out warning about display settings not being optimal
 * 
 * 53    1/09/97 2:16p John
 * took out the "Bing!" message
 * 
 * 52    1/09/97 11:35a John
 * Added some 2d functions to get/put screen images.
 * 
 * 51    1/07/97 2:01p John
 * Fairly fast zbuffering for object sorting.
 * 
 * 50    1/06/97 2:44p John
 * Added in slow (but correct) zbuffering
 * 
 * 49    12/11/96 12:41p John
 * Added new code to draw 3d laser using 2d ellipses.
 * 
 * 48    12/03/96 5:42p John
 * took out debug code.
 * 
 * 47    12/03/96 5:42p John
 * made gr_bitmap clip properly.
 * 
 * 46    12/03/96 5:06p John
 * Added code to draw our own cursor.
 * 
 * 45    11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 44    11/21/96 11:23a John
 * fixed bug with previous.
 * 
 * 43    11/21/96 11:21a John
 * Made gr_get_string_size handle multi line text.
 * Took out gr_get_multiline_string_size
 * 
 * 42    11/21/96 11:06a John
 * fixed warnings.
 * 
 * 41    11/20/96 10:01a Hoffoss
 * A few minor improvements.
 * 
 * 40    11/19/96 2:47p Allender
 * add support for xparent bitmaps in 32bpp
 * 
 * 39    11/18/96 4:46p Allender
 * changed warning message about bit depth to appear when requested
 * doesn't equal actual
 * 
 * 38    11/18/96 4:35p Allender
 * new 16bpp gradient functions
 * 
 * 37    11/18/96 3:09p John
 * made gr_clear always clear to black.
 * 
 * 36    11/18/96 1:48p Allender
 * added 16 bit version of shader
 * 
 * 35    11/18/96 12:36p John
 * Added code to dump screen to a PCX file.
 * 
 * 34    11/18/96 11:40a John
 * Added faster gr_set_color method.
 * 
 * 33    11/15/96 11:26a John
 * Added code to clip text to clipping region.
 * 
 * 32    11/15/96 11:26a Allender
 * fixed up 16 bbp version of gr_bitmap
 * 
 * 31    11/14/96 9:11a John
 * Fixed bug that didn't offset the gr_string text by the current clip
 * region's offset.
 * 
 * 30    11/13/96 6:47p John
 * Added gr_flip function.
 * 
 * 29    11/13/96 10:10a John
 * Increases MAX_WIDTH & HEIGHT for Jasen's massive 1600x1200 display.
 * 
 * 28    11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 27    10/30/96 10:36a Lawrance
 * added gr_diamond function
 * 
 * 26    10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

//#include "graphics/GrSoft.h"
#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "graphics/grstub.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/font.h"
#include "graphics/grinternal.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"
#include "debugconsole/dbugfile.h"

// 3dnow stuff
// #include "amd3d.h"

// Includes for different rendering systems
#include "graphics/grd3dsetup.h"
#include "graphics/gropengl.h"

screen gr_screen;

color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;
color_gun Gr_t_red, Gr_t_green, Gr_t_blue, Gr_t_alpha;
color_gun Gr_ta_red, Gr_ta_green, Gr_ta_blue, Gr_ta_alpha;
color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;


ubyte Gr_original_palette[768];		// The palette 
ubyte Gr_current_palette[768];
char Gr_current_palette_name[128] = NOX("none");

// cursor stuff
int Gr_cursor = -1;
int Web_cursor_bitmap = -1;

int Gr_inited = 0;

// cpu types
int Gr_cpu = 0;	
int Gr_amd3d = 0;
int Gr_katmai = 0;
int Gr_mmx = 0;

uint Gr_signature = 0;

float Gr_gamma = 1.8f;
int Gr_gamma_int = 180;
int Gr_gamma_lookup[256];

/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param int *x - x value (width to be scaled), can be NULL
 * @param int *y - y value (height to be scaled), can be NULL
 * @return always true
 */
bool gr_resize_screen_pos(int *x, int *y)
{
	if(gr_screen.custom_size < 0)	return false;

	int div_by_x = (gr_screen.custom_size == GR_1024) ? 1024 : 640;
	int div_by_y = (gr_screen.custom_size == GR_1024) ?  768 : 480;
			
	if(x) {
		(*x) *= gr_screen.max_w;
		(*x) /= div_by_x;
	}

	if(y) {
		(*y) *= gr_screen.max_h;
		(*y) /= div_by_y;
	}

	return true;
}

/**
 *
 * @param int *x - x value (width to be unsacled), can be NULL
 * @param int *y - y value (height to be unsacled), can be NULL
 * @return always true
 */
bool gr_unsize_screen_pos(int *x, int *y)
{
	if(gr_screen.custom_size < 0)	return false;

	int mult_by_x = (gr_screen.custom_size == GR_1024) ? 1024 : 640;
	int mult_by_y = (gr_screen.custom_size == GR_1024) ?  768 : 480;
			
	if(x) {
		(*x) *= mult_by_x;
		(*x) /= gr_screen.max_w;
	}

	if(y) {
		(*y) *= mult_by_y;
		(*y) /= gr_screen.max_h;
	}

	return true;
}

/**
 *
 * @param int *x - x value (width to be unsacled), can be NULL
 * @param int *y - y value (height to be unsacled), can be NULL
 * @return always true
 */
bool gr_unsize_screen_posf(float *x, float *y)
{
	if(gr_screen.custom_size < 0)	return false;

	float mult_by_x = (float) ((gr_screen.custom_size == GR_1024) ? 1024 : 640);
	float mult_by_y = (float) ((gr_screen.custom_size == GR_1024) ?  768 : 480);
			
	if(x) {
		(*x) *= mult_by_x;
		(*x) /= (float) gr_screen.max_w;
	}

	if(y) {
		(*y) *= mult_by_y;
		(*y) /= (float) gr_screen.max_h;
	}

	return true;
}

void gr_close()
{
	if ( !Gr_inited )	return;

	DBUGFILE_OUTPUT_0("About to palette_flush");
	palette_flush();

	switch( gr_screen.mode )	{
#ifdef _WIN32
	case GR_DIRECT3D:		
		gr_d3d_cleanup();
		break;
#endif
	case GR_OPENGL:
		gr_opengl_cleanup();
		break;

	case GR_STUB:break;

	default:
		Int3();		// Invalid graphics mode
	}

	DBUGFILE_OUTPUT_0("About to gr_font_close");
	gr_font_close();

	Gr_inited = 0;
}

//XSTR:OFF
DCF(gr,"Changes graphics mode")
{
#ifndef HARDWARE_ONLY
	int mode = gr_screen.mode;

	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		
		if ( !strcmp( Dc_arg, "d"))	{
			mode = GR_DIRECT3D;
		} else if ( !strcmp( Dc_arg, "o"))	{
			mode = GR_OPENGL;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}

		/*
		if ( mode != gr_screen.mode )	{
			dc_printf( "Setting new video mode...\n" );
			int errcode = gr_init( gr_screen.max_w, gr_screen.max_h, mode );
			if (errcode)	{
				dc_printf( "Error %d.  Graphics unchanged.\n", errcode );
			}
		}
		*/
	}

	if ( Dc_help )	{
		dc_printf( "Usage: gr mode\n" );
		dc_printf( "The options can be:\n" );
		dc_printf( "Macros:  A=software win32 window (obsolete)\n" );
		dc_printf( "         B=software directdraw fullscreen (obsolete)\n" );
		dc_printf( "         D=Direct3d\n" );
		dc_printf( "         G=Glide\n" );
		dc_printf( "         O=OpenGL\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		switch( gr_screen.mode )	{
		case GR_DIRECT3D:
			dc_printf( "Direct3D\n" );
			break;
		case GR_OPENGL:
			dc_printf( "OpenGl\n" );
			break;
		default:
			Int3();		// Invalid graphics mode
		}
	}
#endif
}
//XSTR:ON

// set screen clear color
DCF(clear_color, "set clear color r, g, b")
{
	int r, g, b;

	dc_get_arg(ARG_INT);
	r = Dc_arg_int;
	dc_get_arg(ARG_INT);
	g = Dc_arg_int;
	dc_get_arg(ARG_INT);
	b = Dc_arg_int;

	// set the color
	gr_set_clear_color(r, g, b);
}

void gr_set_palette_internal( char *name, ubyte * palette, int restrict_font_to_128 )
{
	if ( palette == NULL )	{
		// Create a default palette
		int r,g,b,i;
		i = 0;
				
		for (r=0; r<6; r++ )	
			for (g=0; g<6; g++ )	
				for (b=0; b<6; b++ )		{
					Gr_current_palette[i*3+0] = (unsigned char)(r*51);
					Gr_current_palette[i*3+1] = (unsigned char)(g*51);
					Gr_current_palette[i*3+2] = (unsigned char)(b*51);
					i++;
				}
		for ( i=216;i<256; i++ )	{
			Gr_current_palette[i*3+0] = (unsigned char)((i-216)*6);
			Gr_current_palette[i*3+1] = (unsigned char)((i-216)*6);
			Gr_current_palette[i*3+2] = (unsigned char)((i-216)*6);
		}
		memmove( Gr_original_palette, Gr_current_palette, 768 );
	} else {
		memmove( Gr_original_palette, palette, 768 );
		memmove( Gr_current_palette, palette, 768 );
	}

//	mprintf(("Setting new palette\n" ));

	if ( Gr_inited )	{
		if (gr_screen.gf_set_palette)	{
			(*gr_screen.gf_set_palette)(Gr_current_palette, restrict_font_to_128 );

			// Since the palette set code might shuffle the palette,
			// reload it into the source palette
			if ( palette )
				memmove( palette, Gr_current_palette, 768 );
		}

		// Update Palette Manager tables
		memmove( gr_palette, Gr_current_palette, 768 );
		palette_update(name, restrict_font_to_128);
	}
}


void gr_set_palette( char *name, ubyte * palette, int restrict_font_to_128 )
{
	char *p;
	palette_flush();
	strcpy( Gr_current_palette_name, name );
	p = strchr( Gr_current_palette_name, '.' );
	if ( p ) *p = 0;
	gr_screen.signature = Gr_signature++;
	gr_set_palette_internal( name, palette, restrict_font_to_128 );
}


//void gr_test();

#if defined( _WIN32 ) && defined( _MSC_VER )

#define CPUID _asm _emit 0fh _asm _emit 0a2h

// -----------------------------------------------------------------------
// Returns cpu type.
void gr_detect_cpu(int *cpu, int *mmx, int *amd3d, int *katmai )
{
	DWORD RegEDX;
	DWORD RegEAX;

	// Set defaults
	*cpu = 0;
	*mmx = 0;
	*amd3d = 0;
	*katmai = 0;

	char cpu_vender[16];
	memset( cpu_vender, 0, sizeof(cpu_vender) );

  _asm {

		// Check for prescence of 
		push	eax
		push	ebx
		push	ecx
		push	edx

		pushfd			// get extended flags
		pop	eax
		mov	ebx, eax		// save current flags
		xor	eax, 200000h	// toggle bit 21
		push	eax			// push new flags on stack
		popfd					// flags updated now in flags
		pushfd			// get extended flags
		pop	eax		// store extended flags in eax
		xor	eax, ebx	// if bit 21 r/w then eax <> 0
		je		no_cpuid		

		mov	eax, 0		// setup CPUID to return vender id
      CPUID           // code bytes = 0fh,  0a2h
		mov	DWORD PTR cpu_vender[0], ebx
		mov	DWORD PTR cpu_vender[4], edx
		mov	DWORD PTR cpu_vender[8], ecx
		
      mov eax, 1      // setup CPUID to return features

      CPUID           // code bytes = 0fh,  0a2h

		mov RegEAX, eax	// family, etc returned in eax
      mov RegEDX, edx	// features returned in edx
		jmp	done_checking_cpuid


no_cpuid:
		mov RegEAX, 4<<8	// family, etc returned in eax
      mov RegEDX, 0		// features returned in edx

done_checking_cpuid:								
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax

	}
	


	//RegEAX	.  Bits 11:8 is family
	*cpu = (RegEAX >>8) & 0xF;

	if ( *cpu < 5 )	{
		*cpu = 4;								// processor does not support CPUID
		*mmx = 0;
	}

	//RegEAX	.  Bits 11:8 is family
	*cpu = (RegEAX >>8) & 0xF;

	// Check for MMX
	BOOL retval = TRUE;
   if (RegEDX & 0x800000)               // bit 23 is set for MMX technology
   {

           __try { _asm emms }          // try executing an MMX instruction "emms"

           __except(EXCEPTION_EXECUTE_HANDLER) { retval = FALSE; }

   } else {
		retval = FALSE;
	}
	if ( retval )	{
		*mmx = 1;			// processor supports CPUID but does not support MMX technology
	}

	// Check for Katmai
   if (RegEDX & (1<<25) )               // bit 25 is set for Katmai technology
   {
		*katmai = 1;
   }

	// Check for Amd 3dnow
	/*
	if ( !stricmp( cpu_vender, NOX("AuthenticAMD")) )	{

		_asm {
			mov eax, 0x80000000      // setup CPUID to return extended number of functions

			CPUID           // code bytes = 0fh,  0a2h

			mov RegEAX, eax	// highest extended function value
		}

		if ( RegEAX > 0x80000000 )	{

			_asm {
				mov eax, 0x80000001      // setup CPUID to return extended flags

				CPUID           // code bytes = 0fh,  0a2h

				mov RegEAX, eax	// family, etc returned in eax
				mov RegEDX, edx	// flags in edx
			}

			if (RegEDX & 0x80000000)               // bit 31 is set for AMD-3D technology
			{
				// try executing some 3Dnow instructions
				__try { 

					float x = (float)1.25;            
					float y = (float)1.25;            
					float z;                      

					_asm {
						movd		mm1, x
						movd		mm2, y                  
						PFMUL(AMD_M1, AMD_M2);               
						movd		z, mm1
						femms
						emms
					}

					int should_be_156 = int(z*100);

					if ( should_be_156 == 156 )	{
						*amd3d = 1;
					}

				}          

				__except(EXCEPTION_EXECUTE_HANDLER) { }
			}

		}		
	}
	*/
}
#else
// TEMP mharris FIXME
void gr_detect_cpu(int *cpu, int *mmx, int *amd3d, int *katmai )
{
	if (cpu)
		*cpu = 0;
	if (mmx)
		*mmx = 0;
	if (amd3d)
		*amd3d = 0;
	if (katmai)
		*katmai = 0;
}
#endif // if defined( _WIN32 ) && defined( _MSC_VER )



// --------------------------------------------------------------------------

// RT Created because D3D8 resolution is chosen in the D3D init function not from the launcher
// Everything but D3D calls this before device initialisation
void gr_init_res(int res, int mode, int max_w, int max_h)
{
	if(Fred_running || Pofview_running)
	{		   
		gr_screen.custom_size = -1;
	} 
	else if(max_w == 640 && max_h == 480) 
	{
		gr_screen.custom_size = -1;
		res = GR_640;
	} 
	else if(max_w == 1024 && max_h == 768) 
	{
		gr_screen.custom_size = -1;
   
		// Hi res vp is not present, use 640x480 art in 1024x768!
		if(res != GR_1024)
			gr_screen.custom_size = res;
	} 
	else 
	{
		// Will fall back to 640x480 if sparky hi res isnt there
		gr_screen.custom_size = res;
	}

	gr_screen.signature = Gr_signature++;
	gr_screen.mode = mode;
	gr_screen.res = res;	
	gr_screen.max_w = max_w;
	gr_screen.max_h = max_h;
	gr_screen.aspect = 1.0f;			// Normal PC screen
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right  = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width  = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;
}

bool gr_init(int res, int mode, int depth, int custom_x, int custom_y)
{
	int first_time = 0;

	gr_detect_cpu(&Gr_cpu, &Gr_mmx, &Gr_amd3d, &Gr_katmai );

	mprintf(( "GR_CPU: Family %d, MMX=%s\n", Gr_cpu, (Gr_mmx?"Yes":"No") ));
	
	if ( !Gr_inited )	
		atexit(gr_close);

	// If already inited, shutdown the previous graphics
	if ( Gr_inited )	{
		switch( gr_screen.mode )	{
#ifdef _WIN32
		case GR_DIRECT3D:
			gr_d3d_cleanup();
			break;
#endif  // ifdef _WIN32

		case GR_OPENGL:
			gr_opengl_cleanup();
			break;
		
		case GR_STUB: break;

		default:
			Int3();		// Invalid graphics mode
		}
	} else {
		first_time = 1;
	}

	D3D_enabled = 0;
	Gr_inited = 1;

	// Moved memset to out here so its not all scrubbed when D3D8 needs to call it again to revise 
	// the mode selection through its own launcher
	memset( &gr_screen, 0, sizeof(screen) );
	gr_init_res(res, mode, custom_x, custom_y);
	gr_screen.bits_per_pixel = depth;
	gr_screen.bytes_per_pixel= depth / 8;

	switch( mode )	{
#ifdef _WIN32
		case GR_DIRECT3D:
			Gr_inited = gr_d3d_init();
			break;
#endif  // ifdef WIN32

		case GR_OPENGL:
			gr_opengl_init();
			break;

		case GR_STUB: 
			gr_stub_init();
			break;

		default:
			Int3();		// Invalid graphics mode
	}

	if(Gr_inited == false) {
		return false;
	}

//  	memmove( Gr_current_palette, Gr_original_palette, 768 );
//  	gr_set_palette_internal(Gr_current_palette_name, Gr_current_palette,0);	
	gr_set_palette_internal(Gr_current_palette_name, NULL, 0);	

	bm_init();
	if ( Gr_cursor == -1 ){
		Gr_cursor = bm_load( "cursor" );
	}

	// load the web pointer cursor bitmap
	if (Web_cursor_bitmap < 0)	{
		int nframes;						// used to pass, not really needed (should be 1)
		Web_cursor_bitmap = bm_load_animation("cursorweb", &nframes);
		Assert(Web_cursor_bitmap >= 0);		// if bitmap didnt load, thats not good (this is protected for in release tho)
	}

	gr_set_color(0,0,0);

	gr_set_clear_color(0, 0, 0);

	// Call some initialization functions
	gr_set_shader(NULL);

	return true;
}

void gr_force_windowed()
{
	if ( !Gr_inited )	return;

	switch( gr_screen.mode )	{
#ifdef _WIN32
		case GR_DIRECT3D:
			break;
#endif  // ifdef WIN32

		case GR_OPENGL:
			extern void opengl_minimize();
			opengl_minimize();
			break;

		case GR_STUB: break;

		default:
			Int3();		// Invalid graphics mode
	}

	if ( Os_debugger_running )
		Sleep(1000);		

}

void gr_activate(int active)
{
	if ( !Gr_inited ) return;

	switch( gr_screen.mode )	{
#ifdef _WIN32
		case GR_DIRECT3D:
			{	
				extern void gr_d3d_activate(int active);
				gr_d3d_activate(active);
				return;
			}
			break;
#endif  // ifdef WIN32

		case GR_OPENGL:
			extern void gr_opengl_activate(int active);
			gr_opengl_activate(active);
			break;

		case GR_STUB: break;

		default:
			Int3();		// Invalid graphics mode
	}

}

// -----------------------------------------------------------------------
// gr_set_cursor_bitmap()
//
// Set the bitmap for the mouse pointer.  This is called by the animating mouse
// pointer code.
//
// The lock parameter just locks basically disables the next call of this function that doesnt
// have an unlock feature.  If adding in more cursor-changing situations, be aware of
// unexpected results. You have been warned.
//
// TODO: investigate memory leak of original Gr_cursor bitmap when this is called
void gr_set_cursor_bitmap(int n, int lock)
{
	static int locked = 0;			
	Assert(n >= 0);

	if (!locked || (lock == GR_CURSOR_UNLOCK)) {
		Gr_cursor = n;
	} else {
		locked = 0;
	}

	if (lock == GR_CURSOR_LOCK) {
		locked = 1;
	}
}

// retrieves the current bitmap
// used in UI_GADGET to save/restore current cursor state
int gr_get_cursor_bitmap()
{
	return Gr_cursor;
}

// new bitmap functions
void gr_bitmap(int x, int y, bool allow_scaling)
{
	int section_x, section_y;	
	int x_line, y_line;
	int w, h;

	if(gr_screen.mode == GR_DIRECT3D){

		bm_get_info(gr_screen.current_bitmap, &w, &h, NULL, NULL, NULL, NULL);

		// get the section as a texture in vram					
		gr_set_bitmap(gr_screen.current_bitmap, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, gr_screen.current_alpha);

		// I will tidy this up later - RT
		if(allow_scaling)
		{
			gr_resize_screen_pos(&x, &y);
			gr_resize_screen_pos(&w, &h);
		}

		// RT draws all hall interface stuff
		g3_draw_2d_poly_bitmap(x, y, w, h, TMAP_FLAG_BITMAP_SECTION);
	}
	else if((gr_screen.mode == GR_OPENGL)) {
		int idx, s_idx;
		// float u_scale, v_scale;
		bitmap_section_info *sections;			

		// render all sections
		bm_get_info(gr_screen.current_bitmap, &w, &h, NULL, NULL, NULL, &sections);

		y_line = 0;
		section_y = 0;

		for(idx=0; idx<sections->num_y; idx++){
			x_line = 0;
			for(s_idx=0; s_idx<sections->num_x; s_idx++){

				// get the section as a texture in vram					
				gr_set_bitmap(gr_screen.current_bitmap, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, gr_screen.current_alpha, s_idx, idx);

				// determine the width and height of this section
				bm_get_section_size(gr_screen.current_bitmap, s_idx, idx, &section_x, &section_y);

				int px1 = x + x_line;
				int py1 = y + y_line;
				int px2 = section_x;
				int py2 = section_y;

				// I will tidy this up later - RT
				if(allow_scaling)
				{
					gr_resize_screen_pos(&px1, &py1);
					gr_resize_screen_pos(&px2, &py2);
				}

				// RT draws all hall interface stuff
			   	g3_draw_2d_poly_bitmap(px1, py1, px2, py2, TMAP_FLAG_BITMAP_SECTION | TMAP_HTL_2D);
				x_line += section_x;
			}
			y_line += section_y;
		}
	}
}

// given endpoints, and thickness, calculate coords of the endpoint
void gr_pline_helper(vector *out, vector *in1, vector *in2, int thickness)
{
	vector slope;	

	// slope of the line	
	if(vm_vec_same(in1, in2)){
		slope = vmd_zero_vector;
	} else {
		vm_vec_sub(&slope, in2, in1);
		float temp = -slope.xyz.x;
		slope.xyz.x = slope.xyz.y;
		slope.xyz.y = temp;
		vm_vec_normalize(&slope);
	}

	// get the points		
	vm_vec_scale_add(out, in1, &slope, (float)thickness);
}

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(vector **pts, int num_pts, int thickness)
{				
	vector s1, s2, e1, e2, dir;
	vector last_e1, last_e2;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};
	int saved_zbuffer_mode, idx;		
	int started_frame = 0;

	// Assert(0);

	// if we have less than 2 pts, bail
	if(num_pts < 2){
		return;
	}	

	extern int G3_count;
	if(G3_count == 0){
		g3_start_frame(1);		
		started_frame = 1;
	}

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	// turn off culling
	gr_set_cull(0);

	// draw each section
	last_e1 = vmd_zero_vector;
	last_e2 = vmd_zero_vector;
	for(idx=0; idx<num_pts-1; idx++){		
		// get the start and endpoints		
		s1 = *pts[idx];														// start 1 (on the line)
		gr_pline_helper(&s2, pts[idx], pts[idx+1], thickness);	// start 2
		e1 = *pts[idx+1];														// end 1 (on the line)
		vm_vec_sub(&dir, pts[idx+1], pts[idx]);		
		vm_vec_add(&e2, &s2, &dir);										// end 2
		
		// stuff coords		
		v[0].sx = (float)ceil(s1.xyz.x);
		v[0].sy = (float)ceil(s1.xyz.y);	
		v[0].sw = 0.0f;
		v[0].u = 0.5f;
		v[0].v = 0.5f;
		v[0].flags = PF_PROJECTED;
		v[0].codes = 0;
		v[0].r = gr_screen.current_color.red;
		v[0].g = gr_screen.current_color.green;
		v[0].b = gr_screen.current_color.blue;

		v[1].sx = (float)ceil(s2.xyz.x);
		v[1].sy = (float)ceil(s2.xyz.y);	
		v[1].sw = 0.0f;
		v[1].u = 0.5f;
		v[1].v = 0.5f;
		v[1].flags = PF_PROJECTED;
		v[1].codes = 0;
		v[1].r = gr_screen.current_color.red;
		v[1].g = gr_screen.current_color.green;
		v[1].b = gr_screen.current_color.blue;

		v[2].sx = (float)ceil(e2.xyz.x);
		v[2].sy = (float)ceil(e2.xyz.y);
		v[2].sw = 0.0f;
		v[2].u = 0.5f;
		v[2].v = 0.5f;
		v[2].flags = PF_PROJECTED;
		v[2].codes = 0;
		v[2].r = gr_screen.current_color.red;
		v[2].g = gr_screen.current_color.green;
		v[2].b = gr_screen.current_color.blue;

		v[3].sx = (float)ceil(e1.xyz.x);
		v[3].sy = (float)ceil(e1.xyz.y);
		v[3].sw = 0.0f;
		v[3].u = 0.5f;
		v[3].v = 0.5f;
		v[3].flags = PF_PROJECTED;
		v[3].codes = 0;				
		v[3].r = gr_screen.current_color.red;
		v[3].g = gr_screen.current_color.green;
		v[3].b = gr_screen.current_color.blue;		

		// draw the polys
		g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);		

		// if we're past the first section, draw a "patch" triangle to fill any gaps
		if(idx > 0){
			// stuff coords		
			v[0].sx = (float)ceil(s1.xyz.x);
			v[0].sy = (float)ceil(s1.xyz.y);	
			v[0].sw = 0.0f;
			v[0].u = 0.5f;
			v[0].v = 0.5f;
			v[0].flags = PF_PROJECTED;
			v[0].codes = 0;
			v[0].r = gr_screen.current_color.red;
			v[0].g = gr_screen.current_color.green;
			v[0].b = gr_screen.current_color.blue;

			v[1].sx = (float)ceil(s2.xyz.x);
			v[1].sy = (float)ceil(s2.xyz.y);	
			v[1].sw = 0.0f;
			v[1].u = 0.5f;
			v[1].v = 0.5f;
			v[1].flags = PF_PROJECTED;
			v[1].codes = 0;
			v[1].r = gr_screen.current_color.red;
			v[1].g = gr_screen.current_color.green;
			v[1].b = gr_screen.current_color.blue;


			v[2].sx = (float)ceil(last_e2.xyz.x);
			v[2].sy = (float)ceil(last_e2.xyz.y);
			v[2].sw = 0.0f;
			v[2].u = 0.5f;
			v[2].v = 0.5f;
			v[2].flags = PF_PROJECTED;
			v[2].codes = 0;
			v[2].r = gr_screen.current_color.red;
			v[2].g = gr_screen.current_color.green;
			v[2].b = gr_screen.current_color.blue;

			g3_draw_poly_constant_sw(3, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);		
		}

		// store our endpoints
		last_e1 = e1;
		last_e2 = e2;
	}

	if(started_frame){
		g3_end_frame();
	}

	// restore zbuffer mode
	gr_zbuffer_set(saved_zbuffer_mode);

	// restore culling
	gr_set_cull(1);		
}

