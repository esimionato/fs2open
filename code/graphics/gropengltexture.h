/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTexture.h $
 * $Revision: 1.16 $
 * $Date: 2005-12-28 22:28:44 $
 * $Author: taylor $
 *
 * This file contains function and structure definitions
 * that are needed for managing texture mapping
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 1.14  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 1.13  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 1.12  2005/09/20 02:46:53  taylor
 * slight speedup for font rendering
 * fix a couple of things that Valgrind complained about
 *
 * Revision 1.11  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 1.10  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.9  2005/04/24 12:56:43  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 1.8  2005/04/23 01:17:09  wmcoolmon
 * Removed GL_SECTIONS
 *
 * Revision 1.7  2005/01/21 08:54:53  taylor
 * slightly better memory management
 *
 * Revision 1.6  2005/01/21 08:25:15  taylor
 * fill in gr_opengl_set_texture_addressing()
 * add support for non-power-of-two textures for cards that have it
 * temporary crash fix from multiple mipmap levels in uncompressed formats
 *
 * Revision 1.5  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 1.4  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.3  2004/07/17 18:43:46  taylor
 * don't use bitmap sections by default, openil_close()
 *
 * Revision 1.2  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.4  2004/04/26 12:43:58  taylor
 * minor fixes, HTL lines, 32-bit texture support
 *
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */


#ifndef _GROPENGLTEXTURE_H
#define _GROPENGLTEXTURE_H

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"


//turns on/off GL_TEXTUREx_ARB
void opengl_switch_arb(int unit, int state);

typedef struct tcache_slot_opengl {
	GLuint	texture_id;
//	GLuint	frameb_handle;
//	GLuint	depthb_handle;
	float	u_scale, v_scale;
	int	bitmap_handle;
	int	size;
	//char	used_this_frame;
	int	time_created;
	ushort	w,h;
	ubyte bpp;
} tcache_slot_opengl;

extern int GL_texture_sections;
extern int GL_texture_ram;
extern int GL_frame_count;
extern int GL_min_texture_width;
extern GLint GL_max_texture_width;
extern int GL_min_texture_height;
extern GLint GL_max_texture_height;
extern int GL_square_textures;
extern int GL_textures_in;
extern int GL_textures_in_frame;
extern int GL_last_bitmap_id;
extern int GL_last_detail;
extern int GL_last_bitmap_type;
extern int GL_last_section_x;
extern int GL_last_section_y;
extern GLint GL_supported_texture_units;
extern int GL_should_preload;
extern ubyte GL_xlat[256];
extern int GL_mipmap_filter;

void opengl_tcache_init(int use_sections);
void opengl_free_texture_slot(int n);
void opengl_tcache_flush();
void opengl_tcache_cleanup();
void opengl_tcache_frame();
int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0, int stage = 0);
void opengl_set_additive_tex_env();
void opengl_set_modulate_tex_env();
void gr_opengl_set_tex_env_scale(float scale);
int gr_opengl_preload(int bitmap_num, int is_aabitmap);
void gr_opengl_preload_init();
GLfloat opengl_get_max_anisotropy();
void opengl_set_anisotropy(GLfloat aniso_value = 0.0f);
void gr_opengl_set_texture_panning(float u, float v, bool enable);
void gr_opengl_set_texture_addressing(int mode);

#endif	//_GROPENGLTEXTURE_H
