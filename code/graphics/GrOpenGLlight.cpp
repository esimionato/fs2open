//code to implement lighting in HT&L opengl

#include <windows.h>


#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengllight.h"
#include "graphics/2d.h"
#include "graphics/gl/gl.h"
#include "graphics/gl/glu.h"
#include "graphics/gl/glext.h"
#include "render/3d.h"

// Variables
opengl_light opengl_lights[MAX_LIGHTS];
bool active_light_list[MAX_LIGHTS];
int currently_enabled_lights[MAX_OPENGL_LIGHTS] = {-1};
bool lighting_is_enabled = true;
int active_gl_lights = 0;
int n_active_gl_lights = 0;

extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;
extern double specular_exponent_value;

int max_gl_lights;


void FSLight2GLLight(opengl_light *GLLight,light_data *FSLight) {

	GLLight->Diffuse.r = FSLight->r;// * FSLight->intensity;
	GLLight->Diffuse.g = FSLight->g;// * FSLight->intensity;
	GLLight->Diffuse.b = FSLight->b;// * FSLight->intensity;
	GLLight->Specular.r = FSLight->spec_r;// * FSLight->intensity;
	GLLight->Specular.g = FSLight->spec_g;// * FSLight->intensity;
	GLLight->Specular.b = FSLight->spec_b;// * FSLight->intensity;
	GLLight->Ambient.r = 0.0f;
	GLLight->Ambient.g = 0.0f;
	GLLight->Ambient.b = 0.0f;
	GLLight->Ambient.a = 1.0f;
	GLLight->Specular.a = 1.0f;
	GLLight->Diffuse.a = 1.0f;


	//If the light is a directional light
	if(FSLight->type == LT_DIRECTIONAL) {
		GLLight->Position.x = -FSLight->vec.xyz.x;
		GLLight->Position.y = -FSLight->vec.xyz.y;
		GLLight->Position.z = -FSLight->vec.xyz.z;
		GLLight->Position.w = 0.0f; //Directional lights in OpenGL have w set to 0 and the direction vector in the position field

		GLLight->Specular.r *= static_light_factor;
		GLLight->Specular.g *= static_light_factor;
		GLLight->Specular.b *= static_light_factor;
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {

		if(FSLight->type == LT_POINT){
			GLLight->Specular.r *= static_point_factor;
			GLLight->Specular.g *= static_point_factor;
			GLLight->Specular.b *= static_point_factor;
		}else{
			GLLight->Specular.r *= static_tube_factor;
			GLLight->Specular.g *= static_tube_factor;
			GLLight->Specular.b *= static_tube_factor;
		}

		GLLight->Position.x = FSLight->vec.xyz.x;
		GLLight->Position.y = FSLight->vec.xyz.y;
		GLLight->Position.z = FSLight->vec.xyz.z; //flipped axis for FS2
		GLLight->Position.w = 1.0f;		

		//They also have almost no radius...
//		GLLight->Range = FSLight->radb +FSLight->rada; //No range function in OpenGL that I'm aware of
		GLLight->ConstantAtten = 0.0f;
		GLLight->LinearAtten = 0.1f;
		GLLight->QuadraticAtten = 0.0f; 
	}

}

void set_opengl_light(int light_num, opengl_light *light)
{
	Assert(light_num < max_gl_lights);
	glLightfv(GL_LIGHT0+light_num, GL_POSITION, &light->Position.x);
	glLightfv(GL_LIGHT0+light_num, GL_AMBIENT, &light->Ambient.r);
	glLightfv(GL_LIGHT0+light_num, GL_DIFFUSE, &light->Diffuse.r);
	glLightfv(GL_LIGHT0+light_num, GL_SPECULAR, &light->Specular.r);
	glLightf(GL_LIGHT0+light_num, GL_CONSTANT_ATTENUATION, light->ConstantAtten);
	glLightf(GL_LIGHT0+light_num, GL_LINEAR_ATTENUATION, light->LinearAtten);
	glLightf(GL_LIGHT0+light_num, GL_QUADRATIC_ATTENUATION, light->QuadraticAtten);
}

//finds the first unocupyed light
void opengl_pre_render_init_lights(){
	for(int i = 0; i<max_gl_lights; i++){
		if(currently_enabled_lights[i] > -1) glDisable(GL_LIGHT0+i);
		currently_enabled_lights[i] = -1;
	}
}


void opengl_change_active_lights(int pos){
	int k = 0;
	int l = 0;
	if(!lighting_is_enabled)return;
	bool move = false;
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix();				
	glLoadIdentity();

//straight cut'n'paste out of gr_opengl_set_view_matrix, but I couldn't use that, since it messes up with the stack depth var
	vector fwd;
	vector *uvec=&Eye_matrix.vec.uvec;

	vm_vec_add(&fwd, &Eye_position, &Eye_matrix.vec.fvec);

	gluLookAt(Eye_position.xyz.x,Eye_position.xyz.y,-Eye_position.xyz.z,
	fwd.xyz.x,fwd.xyz.y,-fwd.xyz.z,
	uvec->xyz.x, uvec->xyz.y,-uvec->xyz.z);

	glScalef(1,1,-1);
	
	for(int i = 0; (i < max_gl_lights) && ((pos * max_gl_lights)+i < active_gl_lights); i++){
		glDisable(GL_LIGHT0+i);
		move = false;
		for(k; k<MAX_LIGHTS && !move; k++){
			int slot = (pos * max_gl_lights)+l;
			if(active_light_list[slot]){
				if(opengl_lights[slot].occupied){
					set_opengl_light(i,&opengl_lights[slot]);
					glEnable(GL_LIGHT0+i);
					currently_enabled_lights[i] = slot;
					move = true;
					l++;
				}
			}
		}
	}
	

	glPopMatrix();

}

int	gr_opengl_make_light(light_data* light, int idx, int priority)
{
//Stub
	return idx;
}

void gr_opengl_modify_light(light_data* light, int idx, int priority)
{
//Stub
}

void gr_opengl_destroy_light(int idx)
{
//Stub
}

void gr_opengl_set_light(light_data *light)
{
	//Init the light
	FSLight2GLLight(&opengl_lights[active_gl_lights],light);
	opengl_lights[active_gl_lights].occupied = true;
	active_light_list[active_gl_lights++] = true;
}

void gr_opengl_reset_lighting()
{
	for(int i = 0; i<MAX_LIGHTS; i++){
		opengl_lights[i].occupied = false;
	}
	for(i=0; i<max_gl_lights; i++){
		glDisable(GL_LIGHT0+i);
		active_light_list[i] = false;
	}
	active_gl_lights =0;
}


void gr_opengl_set_lighting(bool set, bool state)
{
	struct {
			float r,g,b,a;
	} ambient, col;

	lighting_is_enabled = set;

	col.r = col.g = col.b = col.a = set ? 1.0f : 0.0f;  //Adunno why the ambient and diffuse need to be set to 0.0 when lighting is disabled
														//They just do, and that should suffice as an answer
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE, &col.r); //changed to GL_FRONT_AND_BACK, just to make sure
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, &col.r); //changed to GL_FRONT_AND_BACK, just to make sure
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,(float)specular_exponent_value);
	if((gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set){
		ambient.r = ambient.g = ambient.b = ambient.a = gr_screen.current_alpha;
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &ambient.r);
	}else{
		ambient.r = ambient.g = ambient.b = 0.125; // 1/16th of the max value, just like D3D
		ambient.a = 1.0f;
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &ambient.r);
	}

	for(int i = 0; i<max_gl_lights; i++){
		if(currently_enabled_lights[i] > -1)glDisable(GL_LIGHT0+i);
		currently_enabled_lights[i] = -1;
	}

	if(state) {
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHTING);
	}

}