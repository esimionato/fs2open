#include "lab/wmcgui.h"
#include "graphics/2d.h"
#include "hud/hudbrackets.h"
#include "parse/parselo.h"
#include "globalincs/linklist.h"
#include "io/key.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"

//#include "Ship/ship.h"

//Colors
color Color_dark_blue;

//Globals
//*****************************ClassInfoEntry*******************************
ClassInfoEntry::ClassInfoEntry()
{
	Type = CIE_NONE;
	Coords[0] = Coords[1] = INT_MAX;
	for(uint i = 0; i < CIE_NUM_HANDLES; i++)
	{
		Handles[i].Image = -1;
	}
}
ClassInfoEntry::~ClassInfoEntry()
{
	//Unload image handles
	if(Type == CIE_IMAGE_NMCSD || Type == CIE_IMAGE || Type == CIE_IMAGE_BORDER)
	{
		int i = 0;
		int num = sizeof(Handle)/sizeof(int);
		for(; i < num; i++)
		{
			if(Handles[i].Image != -1)
				bm_unload(Handles[i].Image);
		}
	}
}

void ClassInfoEntry::Parse(char* tag, int in_type)
{
	char buf[MAX_FILENAME_LEN];
	strcpy(buf, "+");
	strcat(buf, tag);
	if(in_type != CIE_IMAGE_BORDER)
		strcat(buf, ":");

	if(optional_string(buf))
	{
		if(in_type == CIE_IMAGE || in_type == CIE_IMAGE_NMCSD)
		{
			Type = in_type;
			stuff_string(buf, F_NAME, NULL, sizeof(buf));
			Handles[CIE_HANDLE_N].Image = bm_load(buf);
			if(in_type == CIE_IMAGE_NMCSD)
			{
				if(optional_string("+Mouseover:"))
				{
					stuff_string(buf, F_NAME, NULL, sizeof(buf));
					Handles[CIE_HANDLE_M].Image = bm_load(buf);
				}
				if(optional_string("+Clicked:"))
				{
					stuff_string(buf, F_NAME, NULL, sizeof(buf));
					Handles[CIE_HANDLE_C].Image = bm_load(buf);
				}
				if(optional_string("+Selected:"))
				{
					stuff_string(buf, F_NAME, NULL, sizeof(buf));
					Handles[CIE_HANDLE_S].Image = bm_load(buf);
				}
				if(optional_string("+Disabled:"))
				{
					stuff_string(buf, F_NAME, NULL, sizeof(buf));
					Handles[CIE_HANDLE_D].Image = bm_load(buf);
				}
			}
			if(optional_string("+Coords:"))
				stuff_int_list(Coords, 2, RAW_INTEGER_TYPE);
		}
		else if(in_type == CIE_IMAGE_BORDER)
		{
			if(optional_string("+Top Left:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_TL].Image = bm_load(buf);
			}
			if(optional_string("+Top Mid:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_TM].Image = bm_load(buf);
			}
			if(optional_string("+Top Right:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_TR].Image = bm_load(buf);
			}
			if(optional_string("+Mid Left:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_ML].Image = bm_load(buf);
			}
			if(optional_string("+Mid Right:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_MR].Image = bm_load(buf);
			}
			if(optional_string("+Bottom left:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_BL].Image = bm_load(buf);
			}
			if(optional_string("+Bottom Mid:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_BM].Image = bm_load(buf);
			}
			if(optional_string("+Bottom Right:"))
			{
				stuff_string(buf, F_NAME, NULL, sizeof(buf));
				Handles[CIE_HANDLE_BR].Image = bm_load(buf);
			}
		}
		else if(in_type == CIE_TEXT)
		{
		}
	}

#ifndef NDEBUG
	//Do a little extra checking in debug mode
	//This makes sure the skinnger knows if they did something bad
	if(Type == CIE_IMAGE || Type == CIE_IMAGE_NMCSD)
	{
		int w,h,cw,ch;
		bm_get_info(Handles[0].Image, &w, &h);
		for(uint i = 1; i < CIE_NUM_HANDLES; i++)
		{
			if(Handles[i].Image != -1)
			{
				bm_get_info(Handles[i].Image, &cw, &ch);
				if(cw != w || ch != h)
				{
					Warning(LOCATION, "Grouped image size unequal; Handle number %d under $%s: has a different size than base image type", i, tag);
				}
			}
		}
	}
#endif
}

int ClassInfoEntry::GetCoords(int *x, int *y)
{
	int rval = CIE_GC_NONE_SET;
	if(Coords[0]!=INT_MAX && x!=NULL)
	{
		*x=Coords[0];
		rval |= CIE_GC_X_SET;
	}
	if(Coords[1]!=INT_MAX && x!=NULL)
	{
		*y=Coords[1];
		rval |= CIE_GC_Y_SET;
	}

	return rval;
}

//*****************************GUISystem*******************************
GUISystem::GUISystem(char *section)
{
	GraspedGuiobject=ActiveObject=NULL;
	Guiobjects.Globals=this;
	Status=LastStatus=0;

	//Fill out Guiobjects with some handy info
	Guiobjects.Coords[0] = 0;
	Guiobjects.Coords[1] = 0;
	Guiobjects.Coords[2] = gr_screen.clip_width;
	Guiobjects.Coords[3] = gr_screen.clip_height;
	Guiobjects.ChildCoords[0] = gr_screen.clip_left;
	Guiobjects.ChildCoords[1] = gr_screen.clip_top;
	Guiobjects.ChildCoords[2] = gr_screen.clip_right;
	Guiobjects.ChildCoords[3] = gr_screen.clip_bottom;

	//Make ClassInfo all NULL
	for(uint i = 0; i < GT_NUM_TYPES; i++)
	{
		ClassInfo[i] = NULL;
	}
	ParseClassInfo(section);
}
GUISystem::~GUISystem()
{
	GUIObject* cgp = (GUIObject*)GET_FIRST(&Guiobjects);
	GUIObject* cgp_next;
	for(; cgp != END_OF_LIST(&Guiobjects); cgp = cgp_next)
	{
		cgp_next = (GUIObject*)GET_NEXT(cgp);
		delete cgp;
	}
}

int GUISystem::OnFrame(float frametime, bool doevents, bool clearandflip)
{
	//Set the global status variables for this frame
	LastStatus = Status;
	Status = GST_MOUSE_OVER;

	GUIObject* cgp;
	GUIObject* cgp_prev;
	bool SomethingPressed = false;
	int unused_queue;
	if(clearandflip)
		gr_clear();

	if(NOT_EMPTY(&Guiobjects) && doevents)
	{
		GUIObject* cgp = (GUIObject*)GET_LAST(&Guiobjects);
		
		KeyPressed = game_check_key();

		//Add keyboard event stuff
		if(KeyPressed)
		{
			Status |= GST_KEYBOARD_KEYPRESS;

			if(KeyPressed & KEY_CTRLED)
				Status |= GST_KEYBOARD_CTRL;
			if(KeyPressed & KEY_ALTED)
				Status |= GST_KEYBOARD_ALT;
			if(KeyPressed & KEY_SHIFTED)
				Status |= GST_KEYBOARD_SHIFT;
		}

		//Add mouse event stuff
		if(mouse_down(MOUSE_LEFT_BUTTON))
			Status |= GST_MOUSE_LEFT_BUTTON;
		else if(mouse_down(MOUSE_MIDDLE_BUTTON))
			Status |= GST_MOUSE_MIDDLE_BUTTON;
		else if(mouse_down(MOUSE_RIGHT_BUTTON))
			Status |= GST_MOUSE_RIGHT_BUTTON;

		//Now that we are done setting status, add all that stuff to the unused queue
		//Children will be changing this function as they do stuff with statuses
		unused_queue = Status;

		mouse_get_pos(&MouseX, &MouseY);

		//Handle any grasped object (we are in the process of moving this)
		if(GraspedGuiobject != NULL)
		{
			if(Status & GraspingButton)
			{
				SomethingPressed = true;
				GraspedGuiobject->SetPosition(MouseX - GraspedDiff[0], MouseY - GraspedDiff[1]);
			}
			else
			{
				GraspedGuiobject = NULL;
			}
		}

		//Pass the status on
		for(cgp = (GUIObject*)GET_LAST(&Guiobjects); cgp != END_OF_LIST(&Guiobjects); cgp = cgp_prev)
		{
			//In case an object deletes itself
			cgp_prev = (GUIObject*)GET_PREV(cgp);
			cgp->LastStatus = cgp->Status;
			cgp->Status = 0;

			//If we are moving something, nothing else can get click events
			if(GraspedGuiobject == NULL)
			{
				if(GetMouseX() >= cgp->Coords[0]
					&& GetMouseX() <= cgp->Coords[2]
					&& GetMouseY() >= cgp->Coords[1]
					&& GetMouseY() <= cgp->Coords[3])
				{
					cgp->Status |= (Status & GST_MOUSE_STATUS);
					if((Status & GST_MOUSE_LEFT_BUTTON) || (Status & GST_MOUSE_RIGHT_BUTTON) || (Status & GST_MOUSE_MIDDLE_BUTTON))
					{
						SomethingPressed = true;
					}
				}
			}
			else if(cgp == GraspedGuiobject)
			{
				GraspedGuiobject->Status = (Status & GST_MOUSE_STATUS);
			}

			if(cgp == GET_LAST(&Guiobjects))
			{
				cgp->Status |= (Status & GST_KEYBOARD_STATUS);
			}

			cgp->OnFrame(frametime, &unused_queue);
		}
	}

	//Draw now. This prevents problems from an object deleting itself or moving around in the list
	for(cgp = (GUIObject*)GET_LAST(&Guiobjects); cgp != END_OF_LIST(&Guiobjects); cgp = (GUIObject*)GET_PREV(cgp))
	{
		if(!doevents)
			cgp->Status = 0;
		cgp->OnDraw(frametime);
	}

	if(clearandflip)
		gr_flip();

	if(SomethingPressed)
		return GSDF_SOMETHINGPRESSED;
	else
		return GSDF_NOTHINGPRESSED;
}

GUIObject* GUISystem::Add(GUIObject* cgp)
{
	if(cgp == NULL)
		return NULL;

	Assert(this != NULL);

	//Add to the end of the list
	list_append(&Guiobjects, cgp);
	cgp->Globals = this;

	//For skinning
	cgp->SetCIEHandle();
	//In case we need to resize
	cgp->CalculateSize();

	return cgp;
}

void GUISystem::SetActiveObject(GUIObject *cgp)
{
	if(cgp != NULL)
	{
		ActiveObject = cgp;
		//Move everything to the end of the list
		//This way, it gets precedence
		while(cgp->Parent != NULL)
		{
			list_move_append(&cgp->Parent->Children, cgp);
			cgp = cgp->Parent;
		}

		//Eventually, we make this last guy the last object in Guiobjects
		list_move_append(&Guiobjects, cgp);
	}
}

void GUISystem::SetGraspedObject(GUIObject *cgp, int button)
{
	//Protect thyself!
	if(cgp == NULL)
		return;

	GraspedGuiobject = cgp;
	GraspingButton = button;
	GraspedDiff[0] = MouseX - cgp->Coords[0];
	GraspedDiff[1] = MouseY - cgp->Coords[1];
}

void GUISystem::ParseClassInfo(char* section)
{
	ClassInfoEntry *cie;
	unsigned int i;
	for(i = 0; i < GT_NUM_TYPES; i++)
	{
		if(ClassInfo[i] != NULL)
		{
			Warning(LOCATION, "Class info is being parsed twice");
			delete[] ClassInfo[i];
			ClassInfo[i] = NULL;
		}
	}

	lcl_ext_open();
	if(setjmp(parse_abort) != 0)
	{
		nprintf(("Warning", "Unable to parse %s!", section));
		lcl_ext_close();
		return;
	}
	read_file_text("interface.tbl");
	reset_parse();

	//Read the lab section
	char *buf = new char[strlen(section) + 2];
	strcpy(buf, "#");
	strcat(buf, section);
	skip_to_string(buf);
	delete[] buf;
	if(optional_string("$Window"))
	{
		//Create this class section
		ClassInfo[GT_WINDOW] = new ClassInfoEntry[WCI_NUM_ENTRIES];
		cie = ClassInfo[GT_WINDOW];

		//Fill it out
		cie[WCI_CAPTION].Parse("Caption", CIE_IMAGE_NMCSD);
		cie[WCI_HIDE].Parse("Hider", CIE_IMAGE_NMCSD);
		cie[WCI_CLOSE].Parse("Closer", CIE_IMAGE_NMCSD);
		cie[WCI_BODY].Parse("Body", CIE_IMAGE);
		cie[WCI_BORDER].Parse("Border", CIE_IMAGE_BORDER);
	}
	required_string("#End");

	lcl_ext_close();
}

void GUISystem::DestroyClassInfo()
{
	for(uint i = 0; i < GT_NUM_TYPES; i++)
	{
		if(ClassInfo[i] != NULL)
		{
			delete[] ClassInfo[i];
			ClassInfo[i] = NULL;
		}
	}
}

//*****************************GUIObject*******************************
GUIObject::GUIObject(int x_coord, int y_coord, int x_width, int y_height, int in_style)
{
	next = prev = this;
	LastStatus = Status = 0;
	Parent = NULL;
	//Globals = NULL;
	CloseFunction = NULL;

	//These would make no sense
	if(x_coord < 0 || y_coord < 0 || x_width == 0 || y_height == 0)
		return;

	Coords[0] = x_coord;
	Coords[1] = y_coord;
	Coords[2] = x_coord + x_width;
	Coords[3] = y_coord + y_height;

	Style = in_style;

	if(x_width > 0)
		Style |= GS_NOAUTORESIZEX;
	if(y_height > 0)
		Style |= GS_NOAUTORESIZEY;

	Type = GT_NONE;
	InfoEntry = NULL;
}

GUIObject::~GUIObject()
{
	if(CloseFunction != NULL)
		CloseFunction(this);
	//Set these properly
	next->prev = prev;
	prev->next = next;
	DeleteChildren();
}

void GUIObject::DeleteChildren(GUIObject* exception)
{
	GUIObject* cgp = (GUIObject*)GET_FIRST(&Children);
	GUIObject* cgp_next;
	for(; cgp != END_OF_LIST(&Children); cgp = cgp_next)
	{
		cgp_next = (GUIObject*)GET_NEXT(cgp);
		delete cgp;
	}
}
/*
GUIObject* GUIObject::Add(GUIObject* cgp)
{
	if(cgp == NULL)
		return NULL;

	Assert(this != NULL);

	//Add to the end of the list
	list_append(&(Globals->Guiobjects), cgp);
	cgp->Globals = Globals;	//Because the head element has its "Globals" variable set, we can get away with this

	//In case we need to resize
	cgp->CalculateSize();

	return cgp;
}*/

GUIObject* GUIObject::AddChild(GUIObject* cgp)
{
	if(cgp == NULL)
		return NULL;

	//Add to end of child list
	list_append(&Children, cgp);

	cgp->Parent = this;
	cgp->Globals = Globals;

	//Update coordinates (Should be relative x/y and width/height) to absolute coordinates
	cgp->Coords[0] += ChildCoords[0];
	cgp->Coords[1] += ChildCoords[1];
	cgp->Coords[2] += ChildCoords[0];
	cgp->Coords[3] += ChildCoords[1];

	//For skinning
	cgp->SetCIEHandle();
	//In case we need to resize
	cgp->CalculateSize();
	
	return cgp;
}

void GUIObject::OnDraw(float frametime)
{
	DoDraw(frametime);
	if(!(Style & GS_HIDDEN))
	{
		GUIObject *cgp_prev;
		for(GUIObject* cgp = (GUIObject*)GET_LAST(&Children); cgp != END_OF_LIST(&Children); cgp = cgp_prev)
		{
			cgp_prev = (GUIObject*)GET_PREV(cgp);
			cgp->OnDraw(frametime);
		}
	}
}

int GUIObject::OnFrame(float frametime, int *unused_queue)
{
	int rval = OF_TRUE;

	GUIObject *cgp_prev;	//Elements will move themselves to the end of the list if they become active
	for(GUIObject *cgp = (GUIObject*)GET_LAST(&Children); cgp != END_OF_LIST(&Children); cgp = cgp_prev)
	{
		cgp_prev = (GUIObject*)GET_PREV(cgp);
		cgp->LastStatus = cgp->Status;
		cgp->Status = 0;

		if(Status & GST_MOUSE_OVER)
		{
			if(Globals->GetMouseX() >= cgp->Coords[0]
				&& Globals->GetMouseX() <= cgp->Coords[2]
				&& Globals->GetMouseY() >= cgp->Coords[1]
				&& Globals->GetMouseY() <= cgp->Coords[3])
			{
				cgp->Status |= (Status & GST_MOUSE_STATUS);
			}
		}
		if(cgp == GET_LAST(&Children))
		{
			cgp->Status |= (Status & GST_KEYBOARD_STATUS);
		}
		cgp->OnFrame(frametime, &Status);
	}

	if(Status)
	{
		//MOUSE OVER
		if(Status & GST_MOUSE_OVER)
		{
			rval = DoMouseOver(frametime);

			if(rval == OF_DESTROYED)
				return OF_DESTROYED;
			else if(rval == OF_TRUE)
				(*unused_queue) &= ~GST_MOUSE_OVER;
		}

		//MOUSE DOWN
		if(Status & GST_MOUSE_PRESS)
		{
			rval = DoMouseDown(frametime);

			if(rval == OF_DESTROYED)
				return OF_DESTROYED;
			else if(rval == OF_TRUE)
				(*unused_queue) &= ~(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON);
		}

		//MOUSE UP
		if((LastStatus & GST_MOUSE_PRESS) && !(Globals->GetStatus() & GST_MOUSE_PRESS))
		{
			rval = DoMouseUp(frametime);

			if(rval == OF_DESTROYED)
				return OF_DESTROYED;
			else if(rval == OF_TRUE)
				(*unused_queue) &= ~(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON);
		}

		//KEY STATE
		if((Status & GST_KEYBOARD_CTRL) || (Status & GST_KEYBOARD_ALT) || (Status & GST_KEYBOARD_SHIFT))
		{
			rval = DoKeyState(frametime);

			if(rval == OF_DESTROYED)
				return OF_DESTROYED;
			else if(rval == OF_TRUE)
				(*unused_queue) &= ~(GST_KEYBOARD_CTRL | GST_KEYBOARD_ALT | GST_KEYBOARD_SHIFT);
		}

		//KEYPRESS
		if(Status & GST_KEYBOARD_KEYPRESS)
		{
			rval = DoKeyPress(frametime);

			if(rval == OF_DESTROYED)
				return OF_DESTROYED;
			else if(rval == OF_TRUE)
				(*unused_queue) &= ~GST_KEYBOARD_KEYPRESS;
		}
	}

	if(!(Status & GST_MOUSE_OVER) && (LastStatus & GST_MOUSE_OVER))
	{
		rval = DoMouseOut(frametime);
	}

	if(rval == OF_DESTROYED)
		return OF_DESTROYED;

	/*if(!child_keypress && (Status & GST_KEY_PRESSED))
		rval = DoKeyPress(Globals->CurrentKeyChar,frametime);*/

	return DoFrame(frametime);
}

void GUIObject::OnMove(int dx, int dy)
{
	Coords[0]+=dx;
	Coords[1]+=dy;
	Coords[2]+=dx;
	Coords[3]+=dy;
	ChildCoords[0]+=dx;
	ChildCoords[1]+=dy;
	ChildCoords[2]+=dx;
	ChildCoords[3]+=dy;

	for(GUIObject *cgp = (GUIObject*)GET_FIRST(&Children); cgp != END_OF_LIST(&Children); cgp = (GUIObject*)GET_NEXT(cgp))
	{
		cgp->OnMove(dx, dy);
	}

	DoMove(dx, dy);
}

int GUIObject::GetCIECoords(int id, int *x, int *y)
{
	if(InfoEntry!=NULL)
	{
		int rv = InfoEntry[id].GetCoords(x,y);
		if(rv & CIE_GC_X_SET)
		{
			if(*x < 0)
				*x += Coords[2];
			else
				*x += Coords[0];
		}
		if(rv & CIE_GC_Y_SET)
		{
			if(*y < 0)
				*y += Coords[3];
			else
				*y += Coords[1];
		}

		return rv;
	}

	return CIE_GC_NONE_SET;
}

//Call this after an object's type is set to enable the GUIObject CIE functions
void GUIObject::SetCIEHandle()
{
	InfoEntry=Globals->GetClassInfo(Type);
}

void GUIObject::SetPosition(int x, int y)
{
	int dx, dy;
	dx = x - Coords[0];
	dy = y - Coords[1];

	if(dx == 0 && dy == 0)
		return;

	if(Parent != NULL)
	{
		//Don't let stuff get moved outside client window
		if(Coords[2] + dx > Parent->Coords[2])
			dx = Parent->Coords[2] - Coords[2];
		if(Coords[3] + dy > gr_screen.clip_bottom)
			dy = Parent->Coords[3] - Coords[3];

		//Check these last, so we're sure we can move stuff around still
		if(Coords[0] + dx < gr_screen.clip_left)
			dx = Parent->Coords[0] - Coords[0];
		if(Coords[1] + dy < gr_screen.clip_top)
			dy = Parent->Coords[1] - Coords[1];
	}

	OnMove(dx, dy);
}

//*****************************Window*******************************
void Window::CalculateSize()
{
	int w, h;


	//Determine left border's width
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML), &BorderSizes[0], NULL);
	else
		BorderSizes[0] = W_BORDERHEIGHT;

	//Determine right border's width
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR), &BorderSizes[2], NULL);
	else
		BorderSizes[2] = W_BORDERWIDTH;

	//Determine top border height
	bool custom_top = true;
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL), NULL, &BorderSizes[1]);
	else if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), NULL, &BorderSizes[1]);
	else if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM), NULL,  &BorderSizes[1]);
	else
	{
		custom_top = false;
		BorderSizes[1] = W_BORDERHEIGHT;
	}

	//Determine bottom border height
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), NULL, &BorderSizes[3]);
	else if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR), NULL, &BorderSizes[3]);
	else if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM), NULL,  &BorderSizes[3]);
	else
		BorderSizes[3] = W_BORDERHEIGHT;


	//Determine corner sizes
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[0], NULL);
	else
		CornerWidths[0] = BorderSizes[0];

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), &CornerWidths[1], NULL);
	else
		CornerWidths[1] = BorderSizes[2];

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[2], NULL);
	else
		CornerWidths[2] = BorderSizes[0];

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR) != -1)
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[3], NULL);
	else
		CornerWidths[3] = BorderSizes[2];


	//Do child stuff
	ChildCoords[0] = Coords[0] + BorderSizes[0];
	if(custom_top || (Style & WS_NOTITLEBAR))
		ChildCoords[1] = Coords[1] + BorderSizes[1];	//Set earlier on
	else
		ChildCoords[1] = Coords[1] + gr_get_font_height() + 5 + 3*W_BORDERHEIGHT;
	ChildCoords[2] = Coords[2] - BorderSizes[2];
	ChildCoords[3] = Coords[3] - BorderSizes[3];

	if(!(Style & GS_NOAUTORESIZEX))
		Coords[2] = 0;
	if(!(Style & GS_NOAUTORESIZEY))
		Coords[3] = 0;

	for(GUIObject *cgp = (GUIObject*)GET_FIRST(&Children); cgp != END_OF_LIST(&Children); cgp = (GUIObject*)GET_NEXT(cgp))
	{
		//SetPosition children around so they're within the upper-left corner of dialog
		if(cgp->Coords[0] < ChildCoords[0])
			cgp->SetPosition(ChildCoords[0], cgp->Coords[1] + BorderSizes[0]);
		if(cgp->Coords[1] < ChildCoords[1])
			cgp->SetPosition(cgp->Coords[0], Coords[1] + BorderSizes[1]);

		//Resize window to fit children
		if(cgp->Coords[2] >= ChildCoords[2] && !(Style & GS_NOAUTORESIZEX))
		{
			ChildCoords[2] = cgp->Coords[2];
			Coords[2] = cgp->Coords[2] + BorderSizes[2];
		}
		if(cgp->Coords[3] >= ChildCoords[3] && !(Style & GS_NOAUTORESIZEY))
		{
			ChildCoords[3] = cgp->Coords[3];
			Coords[3] = cgp->Coords[3] + BorderSizes[3];
		}
	}

	//Find caption coordinates
	if(!(Style & WS_NOTITLEBAR))
	{
		gr_get_string_size(&w, &h, (char *)Caption.c_str());
		if(GetCIEImageHandle(WCI_CAPTION) != -1)
			bm_get_info(GetCIEImageHandle(WCI_CAPTION), &w, &h);
		else
			h += 5;

		if(GetCIEImageHandle(WCI_CAPTION) != -1)
		{
			CaptionCoords[0] = Coords[0] + BorderSizes[0];
			CaptionCoords[1] = Coords[1] + BorderSizes[1];
			GetCIECoords(WCI_CAPTION, &CaptionCoords[0], &CaptionCoords[1]);
		}
		else
		{
			CaptionCoords[0] = Coords[0] +(((Coords[2]-Coords[0]) - w) / 2);
			CaptionCoords[1] = Coords[1] + BorderSizes[1];
		}
		CaptionCoords[2] = CaptionCoords[0] + w;
		CaptionCoords[3] = CaptionCoords[1] + h;

		//Find close coordinates now
		if(GetCIEImageHandle(WCI_CLOSE) != -1)
			bm_get_info(GetCIEImageHandle(WCI_CLOSE), &w, &h);
		else
			gr_get_string_size(&w, &h, "X");

		CloseCoords[0] = Coords[2] - w;
		CloseCoords[1] = Coords[1] + BorderSizes[1];
		GetCIECoords(WCI_CLOSE, &CloseCoords[0], &CloseCoords[1]);
		CloseCoords[2] = CloseCoords[0] + w;
		CloseCoords[3] = CloseCoords[1] + h;

		//Find hide coordinates now
		if(GetCIEImageHandle(WCI_HIDE) != -1)
			bm_get_info(GetCIEImageHandle(WCI_HIDE), &w, &h);
		else
			gr_get_string_size(&w, &h, "-");

		HideCoords[0] = CloseCoords[0] - w;
		HideCoords[1] = CloseCoords[1];
		GetCIECoords(WCI_HIDE, &HideCoords[0], &HideCoords[1]);
		HideCoords[2] = HideCoords[0] + w;
		HideCoords[3] = HideCoords[1] + h;
	}

	//Do bitmap stuff
	float num;
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM) != -1)
	{
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM));
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM), &w, &h);
		num = (float)((Coords[2]-CornerWidths[3])-(Coords[0]+CornerWidths[2])) / (float)w;
		BorderRectLists[CIE_HANDLE_BM].resize(1);
		BorderRectLists[CIE_HANDLE_BM][0] = bitmap_rect_list(Coords[0] + CornerWidths[2], Coords[3]-h, fl2i(Coords[0]+CornerWidths[2]+w*num),h,0,0,num,1.0f);
		//gr_bitmap_list(&BorderRectLists[CIE_HANDLE_BM], 1, false);
	}

	if(Parent!=NULL)Parent->CalculateSize();
}

int Window::DoMouseOver(float frametime)
{
	if(Globals->GetMouseX() >= HideCoords[0]
		&& Globals->GetMouseX() <= HideCoords[2]
		&& Globals->GetMouseY() >= HideCoords[1]
		&& Globals->GetMouseY() <= HideCoords[3])
	{
		HideHighlight = true;
	}
	else
		HideHighlight = false;

	if(Globals->GetMouseX() >= CloseCoords[0]
		&& Globals->GetMouseX() <= CloseCoords[2]
		&& Globals->GetMouseY() >= CloseCoords[1]
		&& Globals->GetMouseY() <= CloseCoords[3])
	{
		CloseHighlight = true;
	}
	else
		CloseHighlight = false;

	return OF_TRUE;
}

int Window::DoMouseDown(float frametime)
{
	Globals->SetActiveObject(this);

	if(Style & WS_NONMOVEABLE)
		return OF_TRUE;

	if((Globals->GetGraspedObject() == NULL)
		&& (Status & GST_MOUSE_LEFT_BUTTON)
		&& Globals->GetMouseX() >= CaptionCoords[0]
		&& ((Globals->GetMouseX() < CaptionCoords[2]) || ((Style & WS_NOTITLEBAR) && Globals->GetMouseX() <= Coords[2]))
		&& Globals->GetMouseY() >= CaptionCoords[1]
		&& ((Globals->GetMouseY() <= CaptionCoords[3]) || ((Style & WS_NOTITLEBAR) && Globals->GetMouseY() <= Coords[2])))
		{
			Globals->SetGraspedObject(this, GST_MOUSE_LEFT_BUTTON);
		}

		//All further movement of grasped objects is handled by GUISystem

	return OF_TRUE;
}

int Window::DoMouseUp(float frametime)
{
	if(CloseHighlight)
	{
		delete this;
		return OF_DESTROYED;
	}

	if(HideHighlight)
	{
		if(Style & GS_HIDDEN)
		{
			Coords[3] = UnhiddenHeight;
			Style &= ~GS_HIDDEN;
		}
		else
		{
			UnhiddenHeight = Coords[3];
			Coords[3] = Coords[0] + BorderSizes[1] + BorderSizes[3];
			Style |= GS_HIDDEN;
		}
	}
	return OF_TRUE;
}

int Window::DoMouseOut(float frametime)
{
	HideHighlight = false;
	CloseHighlight = false;
	return OF_TRUE;
}

void Window::DoMove(int dx, int dy)
{
	CloseCoords[0] += dx;
	CloseCoords[1] += dy;
	CloseCoords[2] += dx;
	CloseCoords[3] += dy;

	HideCoords[0] += dx;
	HideCoords[1] += dy;
	HideCoords[2] += dx;
	HideCoords[3] += dy;

	CaptionCoords[0] += dx;
	CaptionCoords[1] += dy;
	CaptionCoords[2] += dx;
	CaptionCoords[3] += dy;
}

void draw_open_rect(int x1, int y1, int x2, int y2, bool resize = false)
{
	gr_line(x1, y1, x2, y1, resize);
	gr_line(x2, y1, x2, y2, resize);
	gr_line(x2, y2, x1, y2, resize);
	gr_line(x1, y2, x1, y1, resize);
}

void Window::DoDraw(float frametime)
{
	gr_set_color_fast(&Color_text_normal);
	int w, h;
	std::vector<bitmap_2d_list> bmlist;
	bitmap_2d_list b2l;
	unsigned int num;
	unsigned int i;
	
	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL) != -1)
	{
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL));
		gr_bitmap(Coords[0], Coords[1], false);
	}

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR) != -1)
	{
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR));
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), &w, NULL);
		gr_bitmap(Coords[2] - w, Coords[1], false);
	}

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL) != -1)
	{
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), NULL, &h);
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL));
		gr_bitmap(Coords[0], Coords[3] - h, false);
	}

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR) != -1)
	{
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR));
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR), &w, &h);
		//gr_bitmap(Coords[2] - w, Coords[3] - h, false);
		gr_bitmap(Coords[2]-w, Coords[3] - h, false);
	}

	if(BorderRectLists[CIE_HANDLE_BM].size())
	{
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM));
		gr_bitmap_list(&BorderRectLists[CIE_HANDLE_BM][0], BorderRectLists[CIE_HANDLE_BM].size(), false);
	}
	else
	{
		gr_line(Coords[0] + BorderSizes[0], Coords[3], Coords[2] - BorderSizes[2], Coords[3]);
	}

	if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM) != -1)
	{
		bmlist.clear();
		gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM));
		bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM), &w, &h);
		num = ((Coords[2]-CornerWidths[1])-(Coords[0]+CornerWidths[0])) / w;
		b2l.w = w;
		b2l.h = h;
		b2l.y = Coords[1];
		for(i = 0; i < num; i++)
		{
			b2l.x = Coords[0] + CornerWidths[0] + w*i;
			bmlist.push_back(b2l);
		}
		gr_bitmap_list(&bmlist[0], bmlist.size(), false);
	}
	else
	{
		gr_line(Coords[0] + BorderSizes[0], Coords[1], Coords[2] - BorderSizes[2], Coords[1]);
	}

	if(!(Style & GS_HIDDEN))
	{
		if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML) != -1)
		{
			bmlist.clear();
			gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML));
			bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML), &w, &h);
			num = ((Coords[3]-BorderSizes[3])-(Coords[1]+BorderSizes[1])) / h;
			b2l.w = w;
			b2l.h = h;
			b2l.x = Coords[0];
			for(i = 0; i < num; i++)
			{
				b2l.y = Coords[1] + BorderSizes[1] + h*i;
				bmlist.push_back(b2l);
			}
			gr_bitmap_list(&bmlist[0], bmlist.size(), false);
			//gr_bitmap(Coords[0], Coords[1] + BorderSizes[1]);
		}
		else
		{
			gr_line(Coords[0], Coords[1] + BorderSizes[1], Coords[0], Coords[3] - BorderSizes[3]);
		}

		if(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR) != -1)
		{
			bmlist.clear();
			gr_set_bitmap(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR));
			bm_get_info(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR), &w, &h);
			num = ((Coords[3]-BorderSizes[3])-(Coords[1]+BorderSizes[1])) / h;
			b2l.w = w;
			b2l.h = h;
			b2l.x = Coords[2]-w;
			for(i = 0; i < num; i++)
			{
				b2l.y = Coords[1] + BorderSizes[1] + h*i;
				bmlist.push_back(b2l);
			}
			gr_bitmap_list(&bmlist[0], bmlist.size(), false);
			//gr_bitmap(Coords[2] - w, Coords[1] + BorderSizes[1], false);
		}
		else
		{
			gr_line(Coords[2], Coords[1] + BorderSizes[1], Coords[2], Coords[3] - BorderSizes[3]);
		}
	}

	if(!(Style & WS_NOTITLEBAR))
	{
		//Draw the caption background
		if(GetCIEImageHandle(WCI_CAPTION) != -1)
		{
			gr_set_bitmap(GetCIEImageHandle(WCI_CAPTION));
			gr_bitmap(CaptionCoords[0], CaptionCoords[1], false);
		}
		else
		{
			draw_open_rect(Coords[0], Coords[1], Coords[2], CaptionCoords[3]);
		}

		//Close button
		if(GetCIEImageHandle(WCI_CLOSE) != -1)
		{
			if(CloseHighlight && GetCIEImageHandle(WCI_CLOSE, CIE_HANDLE_M) != -1)
				gr_set_bitmap(GetCIEImageHandle(WCI_CLOSE, CIE_HANDLE_M));
			else
				gr_set_bitmap(GetCIEImageHandle(WCI_CLOSE));

			gr_bitmap(CloseCoords[0], CloseCoords[1], false);
		}
		else
		{
			if(CloseHighlight)
				gr_set_color_fast(&Color_text_active);
			else
				gr_set_color_fast(&Color_text_normal);
		
			gr_string(CloseCoords[0], CloseCoords[1], "X", false);
		}
		

		//Hide button
		if(GetCIEImageHandle(WCI_HIDE) != -1)
		{
			if(HideHighlight && GetCIEImageHandle(WCI_HIDE, CIE_HANDLE_M) != -1)
				gr_set_bitmap(GetCIEImageHandle(WCI_HIDE, CIE_HANDLE_M));
			else
				gr_set_bitmap(GetCIEImageHandle(WCI_HIDE));
			gr_bitmap(HideCoords[0], HideCoords[1],false);
		}
		else
		{
			if(HideHighlight)
				gr_set_color_fast(&Color_text_active);
			else
				gr_set_color_fast(&Color_text_normal);

			gr_string(HideCoords[0], HideCoords[1], "-", false);
		}

		//Caption text
		gr_set_color_fast(&Color_text_normal);

		gr_string(CaptionCoords[0], CaptionCoords[1], (char *)Caption.c_str(), false);
	}
}

Window::Window(std::string in_caption, int x_coord, int y_coord, int x_width, int y_width, int in_style)
:GUIObject(x_coord,y_coord,x_width,y_width,in_style)
{
	Caption = in_caption;

	CloseHighlight = false;
	HideHighlight = false;

	//Set the type
	Type = GT_WINDOW;
}

//*****************************Button*******************************

Button::Button(std::string in_caption, int x_coord, int y_coord, void (*in_function)(Button *caller), int x_width, int y_height, int in_style)
:GUIObject(x_coord, y_coord, x_width, y_height, in_style)
{
	Caption = in_caption;

	function = in_function;

	IsDown = false;

	//Set the type
	Type = GT_BUTTON;
}

void Button::CalculateSize()
{
	int w, h;
	gr_get_string_size(&w, &h, (char *)Caption.c_str());
	if(!(Style & GS_NOAUTORESIZEX))
	{
		Coords[2] = Coords[0] + w;
	}
	if(!(Style & GS_NOAUTORESIZEY))
	{
		Coords[3] = Coords[1] + h;
	}

	if(Parent!=NULL)Parent->CalculateSize();
}

void Button::DoDraw(float frametime)
{
	if(Status == 0)
		gr_set_color_fast(&Color_text_normal);
	else if(Status & GST_MOUSE_OVER)
		gr_set_color_fast(&Color_text_active);
	else
		gr_set_color_fast(&Color_text_selected);
	draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3], false);

	int half_x, half_y;
	gr_get_string_size(&half_x, &half_y, (char *)Caption.c_str());
	half_x = Coords[0] +(((Coords[2]-Coords[0]) - half_x) / 2);
	half_y = Coords[1] +(((Coords[3]-Coords[1]) - half_y) / 2);
	gr_string(half_x, half_y, (char *)Caption.c_str(), false);
	gr_set_color_fast(&Color_red);
}

int Button::DoMouseDown(float frametime)
{
	Globals->SetActiveObject(this);

	IsDown = !IsDown;
	return OF_TRUE;
}

int Button::DoMouseUp(float frametime)
{
	if(function != NULL)
	{
		function(this);
	}
	if(!(Style & BS_STICKY))
		IsDown = false;

	return OF_TRUE;
}

int Button::DoMouseOut(float frametime)
{
	if(!(Style & BS_STICKY))
		IsDown = false;

	return OF_TRUE;
}

//*****************************Menu*******************************
TreeItem::TreeItem()
:LinkedList()
{
	Function = NULL;
	Data=NULL;
	ShowThis=true;
	ShowChildren=false;
	DeleteData=true;
	memset(Coords, 0, sizeof(Coords));
}
TreeItem::~TreeItem()
{
	//Because I don't think the default LinkedList destructor is called
	prev->next=next;
	next->prev=prev;
	
	LinkedList* cgp = GET_FIRST(&Children);
	LinkedList* cgp_next;
	for(; cgp != END_OF_LIST(&Children); cgp = cgp_next)
	{
		cgp_next = GET_NEXT(cgp);
		delete cgp;
	}

	if(Data != NULL && DeleteData)
		delete Data;	//A good idea
}

void Tree::CalcItemsSize(TreeItem *items, int DrawData[4])
{
	//DrawData
	//0 - indent
	//1 - largest width so far
	//2 - total height so far
	//3 - Are we done?
	int w, h;

	int temp_largest = DrawData[1];
	for(TreeItem* tip = (TreeItem*)GET_FIRST(items); tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip))
	{
		if(!DrawData[3])
		{
			gr_get_string_size(&w, &h, (char*)tip->Name.c_str());
			if((w + DrawData[0]) > temp_largest)
					temp_largest = w + DrawData[0];

			//This should really be moved to where we handle the scrolling code
			//I'm not doing that right now. :)
			//Or at least this should be called from that
			if(((DrawData[2] + h) > Coords[3] && (Style & GS_NOAUTORESIZEY))
				|| (temp_largest > Coords[2] && (Style & GS_NOAUTORESIZEX))
				//|| ((Coords[0] + w) > Globals->Guiobjects.Coords[2])
				//|| ((Coords[1] + h) > Globals->Guiobjects.Coords[3])
				)
			{
				DrawData[3] = 1;
				tip->ShowThis = false;
			}
			else
				tip->ShowThis = true;

			//Do we care?
			if(tip->ShowThis)
			{
				if((w + DrawData[0]) > DrawData[1])
					DrawData[1] = w + DrawData[0];

				tip->Coords[0] = Coords[0] + DrawData[0];
				tip->Coords[1] = Coords[1] + DrawData[2];
				tip->Coords[2] = Coords[0] + DrawData[0] + w;
				tip->Coords[3] = Coords[1] + DrawData[2] + h;

				DrawData[2] += h;

				if(NOT_EMPTY(&tip->Children) && tip->ShowChildren)
				{
					//Indent
					DrawData[0] += TI_INDENT_PER_LEVEL;
					CalcItemsSize((TreeItem*)&tip->Children, DrawData);
					DrawData[0] -= TI_INDENT_PER_LEVEL;
				}
			}
		}
		if(DrawData[3] || !tip->ShowThis)
		{
			tip->ShowThis = false;
			tip->ShowChildren = false;
			tip->Coords[0] = tip->Coords[1] = tip->Coords[2] = tip->Coords[3] = -1;
		}
	}
}

void Tree::CalculateSize()
{
	//DrawData
	//0 - indent
	//1 - largest width so far
	//2 - total height so far
	//3 - Are we done? 0=no,1=yes
	int DrawData[4] = {TI_INITIAL_INDENT,0,TI_INITIAL_INDENT_VERTICAL,0};
	CalcItemsSize(&Items, DrawData);

	//CalcItemsSize takes into account limited height/width
	if(!(Style & GS_NOAUTORESIZEX))
	{
		Coords[2] = Coords[0] + DrawData[1] + TI_BORDER_WIDTH;
	}
	if(!(Style & GS_NOAUTORESIZEY))
	{
		Coords[3] = Coords[1] + DrawData[2] + TI_BORDER_HEIGHT;
		if(Parent != NULL)
		{
			if(DrawData[2] >= Parent->ChildCoords[3])
			{
				//The parent must resize.
			}
		}
	}

	if(!(Style & GS_NOAUTORESIZEX))
	{
		Coords[2] = Coords[0] + DrawData[1];
	}

	if(Parent!=NULL)Parent->CalculateSize();
}

Tree::Tree(int x_coord, int y_coord, void* in_associateditem, int x_width, int y_height, int in_style)
:GUIObject(x_coord, y_coord, x_width, y_height, in_style)
{

	AssociatedItem = in_associateditem;

	//Set the type
	Type = GT_MENU;
}

void Tree::DrawItems(TreeItem *items)
{
	for(TreeItem* tip = (TreeItem*)GET_FIRST(items); tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip))
	{
		if(tip->ShowThis)
		{
			if(SelectedItem == tip)
				gr_set_color_fast(&Color_text_selected);
			else if(HighlightedItem == tip)
				gr_set_color_fast(&Color_text_active);
			else
				gr_set_color_fast(&Color_text_normal);

			gr_string(tip->Coords[0], tip->Coords[1], (char*)tip->Name.c_str(), false);

			if(NOT_EMPTY(&tip->Children) && tip->ShowChildren)
			{
				DrawItems((TreeItem*)&tip->Children);
			}
		}
	}
}

void Tree::DoDraw(float frametime)
{

	if(EMPTY(&Items))
		return;

	DrawItems(&Items);

	gr_set_color_fast(&Color_text_normal);
	draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3]);
}

TreeItem* Tree::HitTest(TreeItem *items)
{
	TreeItem *hti = NULL;	//Highlighted item

	for(TreeItem *tip = (TreeItem*)GET_FIRST(items);tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip))
	{
		if(Globals->GetMouseX() >= tip->Coords[0]
			&& Globals->GetMouseX() <= tip->Coords[2]
			&& Globals->GetMouseY() >= tip->Coords[1]
			&& Globals->GetMouseY() <= tip->Coords[3])
			{
				hti = tip;
			}

			if(hti != NULL)
				return hti;
			else if(NOT_EMPTY(&tip->Children) && tip->ShowChildren)
				hti = HitTest((TreeItem*)&tip->Children);

			if(hti != NULL)
				return hti;
	}

	return hti;
}

int Tree::DoMouseOver(float frametime)
{
	HighlightedItem = HitTest(&Items);

	return OF_TRUE;
}

int Tree::DoMouseDown(float frametime)
{
	Globals->SetActiveObject(this);
	return OF_TRUE;
}

int Tree::DoMouseUp(float frametime)
{
	Globals->SetActiveObject(this);

	if(HighlightedItem != NULL)
	{
		SelectedItem = HighlightedItem;
		SelectedItem->ShowChildren = !SelectedItem->ShowChildren;
		if(SelectedItem->Function != NULL)
		{
			SelectedItem->Function(this);
		}
		CalculateSize();	//Unfortunately
	}

	return OF_TRUE;
}

void Tree::MoveTreeItems(int dx, int dy, TreeItem *items)
{
	for(TreeItem *tip = (TreeItem*)GET_FIRST(items);tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip))
	{
		if(tip->ShowThis)
		{
			tip->Coords[0] += dx;
			tip->Coords[1] += dy;
			tip->Coords[2] += dx;
			tip->Coords[3] += dy;

			if(NOT_EMPTY(&tip->Children) && tip->ShowChildren)
				MoveTreeItems(dx, dy, (TreeItem*)&tip->Children);
		}
	}
}

void Tree::DoMove(int dx, int dy)
{
	MoveTreeItems(dx, dy, &Items);
}

TreeItem* Tree::AddItem(TreeItem *parent, std::string in_name, void *in_data, bool in_delete_data, void (*in_function)(Tree* caller))
{
	TreeItem *ni = new TreeItem;

	ni->Parent = parent;
	ni->Name = in_name;
	ni->Data = in_data;
	ni->DeleteData = in_delete_data;
	ni->Function = in_function;
	if(parent != NULL)
		list_append(&parent->Children, ni);
	else
		list_append(&Items, ni);

	CalculateSize();
	return ni;
}
/*
void Tree::LoadItemList(MenuItem *in_list, unsigned int count)
{
	unsigned int oldcount = Items.size();
	Items.resize(oldcount + count);
	memcpy(&Items[oldcount], in_list, sizeof(MenuItem) * count);

	CalculateSize();
}*/

//*****************************Text*******************************
void Text::CalculateSize()
{
	int width;
	if(Style & GS_NOAUTORESIZEX)
		width = Coords[2] - Coords[0];
	else if(Parent != NULL)
		width = Parent->ChildCoords[2] - Coords[0];
	else
		width = gr_screen.clip_width - Coords[0];

	NumLines = split_str((char*)Content.c_str(), width, LineLengths, LineStartPoints, MAX_TEXT_LINES);

	//Find the shortest width we need to show all the shortened strings
	int longest_width = 0;
	int width_test;
	for(int i = 0; i < NumLines; i++)
	{
		gr_get_string_size(&width_test, NULL, LineStartPoints[i], LineLengths[i]);
		if(width_test > longest_width)
			longest_width = width_test;
	}
	if(longest_width < width)
		width = longest_width;

	//Resize along the Y axis
	if(Style & GS_NOAUTORESIZEY)
	{
		NumLines = (Coords[3] - Coords[1]) / gr_get_font_height();
	}
	else
	{
		Coords[3] = Coords[1] + gr_get_font_height() * NumLines;
		if(Parent != NULL)
		{
			if(Coords[3] >= Parent->ChildCoords[3] && (Parent->Style & GS_NOAUTORESIZEY))
			{
				Coords[3] = Coords[1] + (Parent->ChildCoords[3] - Coords[1]);
				NumLines = (Coords[3] - Coords[1]) / gr_get_font_height();
			}
		}
	}

	if(!(Style & GS_NOAUTORESIZEX))
	{
		Coords[2] = Coords[0] + width;
	}

	if(Style & T_EDITTABLE)
	{
		ChildCoords[0] = Coords[0] + 1;
		ChildCoords[1] = Coords[1] + 1;
		ChildCoords[2] = Coords[2] - 1;
		ChildCoords[3] = Coords[3] - 1;
	}
	else
	{
		ChildCoords[0] = Coords[0];
		ChildCoords[1] = Coords[1];
		ChildCoords[2] = Coords[2];
		ChildCoords[3] = Coords[3];
	}

	//Tell the parent to recalculate its size too
	if(Parent!=NULL)Parent->CalculateSize();
}

int Text::DoMouseDown(float frametime)
{
	//Make this the active object
	if(Style & T_EDITTABLE)
	{
		Globals->SetActiveObject(this);

		return OF_TRUE;
	}
	else
	{
		return OF_FALSE;
	}
}

void Text::DoDraw(float frametime)
{
	if(Globals->GetActiveObject() != this)
		gr_set_color_fast(&Color_text_normal);
	else
		gr_set_color_fast(&Color_text_active);

	if(Style & T_EDITTABLE)
	{
		draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3]);
	}

	//Don't draw anything. Besides, it crashes
	if(!Content.length())
		return;

	int font_height = gr_get_font_height();

	for(int i = 0; i < NumLines; i++)
	{
		gr_string(ChildCoords[0], ChildCoords[1] + (i*font_height), (char*)Content.substr(LineStartPoints[i] - Content.c_str(), LineLengths[i]).c_str(), false);
	}
}

extern int keypad_to_ascii(int c);

int Text::DoKeyPress(float frametime)
{
	if(!(Style & T_EDITTABLE))
		return OF_FALSE;

	switch(Globals->GetKeyPressed())
	{
		case KEY_BACKSP:
			Content = Content.substr(0, Content.length() - 1);
			break;
		case KEY_ENTER:
			if(SaveType & T_ST_ONENTER)
			{
				if(Save())
					Globals->SetActiveObject(Parent);
				else
					Load();
			}
			break;
		case KEY_ESC:
			Load();
			Globals->SetActiveObject(Parent);
			break;
		default:
			{
				//Figure out what key, exactly, we have
				int symbol = keypad_to_ascii(Globals->GetKeyPressed());
				if(symbol == -1)
					symbol = key_to_ascii(Globals->GetKeyPressed());
				//Is it ok with us?
				if(iscntrl(symbol))
				{
					return OF_FALSE;	//Guess not
				}
				else if(SaveType & T_ST_INT)
				{
					if(!isdigit(symbol) && symbol != '-')
						return OF_FALSE;
				}
				else if(SaveType & T_ST_FLOAT)
				{
					if(!isdigit(symbol) && symbol != '.' && symbol != '-')
						return OF_FALSE;
				}
				else if(SaveType & T_ST_UBYTE)
				{
					if(!isdigit(symbol))
						return OF_FALSE;
				}
				else if(!isalnum(symbol) && symbol != ' ')
				{
					return OF_FALSE;
				}

				//We can add the letter to the box. Cheers.
				//If this warning bothers you, go ahead and cast it.
				Content += symbol;
				if(SaveType & T_ST_REALTIME)
				{
					Save();
				}
			}
	}
	CalculateSize();
	return OF_TRUE;
}

Text::Text(std::string in_content, int x_coord, int y_coord, int x_width, int y_height, int in_style)
:GUIObject(x_coord, y_coord, x_width, y_height, in_style)
{
	Content = in_content;

	NumLines = 0;

	SaveType = T_ST_NONE;
	CursorPos = -1;	//Not editing

	Type = GT_TEXT;
}

void Text::SetText(std::string in_content)
{
	Content = in_content;

	CalculateSize();
}

void Text::SetText(int the_int)
{
	char buf[32];
	itoa(the_int, buf, 10);
	Content = buf;

	CalculateSize();
}

void Text::SetText(float the_float)
{
	char buf[32];
	sprintf(buf, "%f", the_float);
	Content = buf;

	CalculateSize();
}

void Text::AddLine(std::string in_line)
{
	Content += in_line;

	CalculateSize();
}

void Text::SetSaveLoc(int *int_ptr, int save_method, int max_value, int min_value)
{
	Assert(int_ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Mmm-hmm.

	SaveType = T_ST_INT;
	if(save_method == T_ST_CLOSE)
		SaveType |= T_ST_CLOSE;
	else if(save_method == T_ST_REALTIME)
		SaveType |= T_ST_REALTIME;
	else if(save_method == T_ST_ONENTER)
		SaveType |= T_ST_ONENTER;

	iSavePointer = int_ptr;
	SaveMax = max_value;
	SaveMin = min_value;
}

void Text::SetSaveLoc(float *ptr, int save_method, float max_value, float min_value)
{
	Assert(ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Causes problems with floats

	SaveType = T_ST_FLOAT;
	if(save_method == T_ST_CLOSE)
		SaveType |= T_ST_CLOSE;
	else if(save_method == T_ST_REALTIME)
		SaveType |= T_ST_REALTIME;
	else if(save_method == T_ST_ONENTER)
		SaveType |= T_ST_ONENTER;

	flSavePointer = ptr;
	flSaveMax = max_value;
	flSaveMin = min_value;
}

void Text::SetSaveLoc(char *ptr, int save_method, uint max_length, uint min_length)
{
	Assert(ptr != NULL);		//Naughty.

	SaveType = T_ST_CHAR;

	if(save_method == T_ST_CLOSE)
		SaveType |= T_ST_CLOSE;
	else if(save_method == T_ST_REALTIME)
		SaveType |= T_ST_REALTIME;
	else if(save_method == T_ST_ONENTER)
		SaveType |= T_ST_ONENTER;

	chSavePointer = ptr;
	uSaveMax = max_length;
	uSaveMin = min_length;
}

void Text::SetSaveStringAlloc(char **ptr, int save_method, int mem_flags, uint max_length, uint min_length)
{
	Assert(ptr != NULL);		//Naughty.

	if(mem_flags == T_ST_NEW)
		SaveType = T_ST_NEW;
	else if(mem_flags == T_ST_MALLOC)
		SaveType = T_ST_MALLOC;
	else
		return;	//This MUST use some sort of mem allocation

	SaveType |= T_ST_CHAR;

	if(save_method == T_ST_CLOSE)
		SaveType |= T_ST_CLOSE;
	else if(save_method == T_ST_REALTIME)
		SaveType |= T_ST_REALTIME;
	else if(save_method == T_ST_ONENTER)
		SaveType |= T_ST_ONENTER;

	chpSavePointer = ptr;
	uSaveMax = max_length;
	uSaveMin = min_length;
}

void Text::SetSaveLoc(ubyte *ptr, int save_method, int max_value, int min_value)
{
	Assert(ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Mmm-hmm.

	SaveType = T_ST_UBYTE;
	if(save_method == T_ST_CLOSE)
		SaveType |= T_ST_CLOSE;
	else if(save_method == T_ST_REALTIME)
		SaveType |= T_ST_REALTIME;
	else if(save_method == T_ST_ONENTER)
		SaveType |= T_ST_ONENTER;

	ubSavePointer = ptr;
	SaveMax = max_value;
	SaveMin = min_value;
}

bool Text::Save()
{
	if(SaveType == T_ST_NONE)
		return false;

	if(SaveType & T_ST_INT)
	{
		int the_int =  atoi(Content.c_str());
		if(the_int <= SaveMax && the_int >= SaveMin)
		{
			*iSavePointer = the_int;
			return true;
		}
	}
	else if(SaveType & T_ST_FLOAT)
	{
		float the_float = (float)atof(Content.c_str());
		if(the_float <= flSaveMax && the_float >= flSaveMin)
		{
			*flSavePointer = the_float;
			return true;
		}
	}
	else if(SaveType & T_ST_CHAR)
	{
		size_t len = Content.length();
		if(SaveType & T_ST_NEW
			&& len < uSaveMax)
		{
			if(*chpSavePointer != NULL)
				delete[] *chpSavePointer;

			*chpSavePointer = new char[len + 1];

			strcpy(*chpSavePointer, Content.c_str());
			return true;
		}
		else if(SaveType & T_ST_MALLOC
			&& len < uSaveMax)
		{
			if(*chpSavePointer != NULL)
				free(*chpSavePointer);

			*chpSavePointer = (char*)malloc(sizeof(char) * (len + 1));

			strcpy(*chpSavePointer, Content.c_str());
			return true;
		}
		else if(len < uSaveMax)
		{
			strcpy(chSavePointer, Content.c_str());
			return true;
		}
	}
	else if(SaveType & T_ST_UBYTE)
	{
		int the_int = atoi(Content.c_str());
		if(the_int <= SaveMax && the_int >= SaveMin)
		{
			*ubSavePointer = (ubyte)the_int;
			return true;
		}
	}

	return false;
}

void Text::Load()
{
	if(SaveType == T_ST_NONE)
		return;

	if(SaveType & T_ST_INT)
		SetText(*iSavePointer);
	else if(SaveType & T_ST_FLOAT)
		SetText(*flSavePointer);
	else if((SaveType & T_ST_CHAR) && (SaveType & (T_ST_MALLOC | T_ST_NEW)))
		SetText(*chpSavePointer);
	else if(SaveType & T_ST_CHAR)
		SetText(chSavePointer);
	else if(SaveType & T_ST_UBYTE)
		SetText(*ubSavePointer);
	else
		Warning(LOCATION, "Unknown type (or no type) given in Text::Load() - nothing happened.");
}

//*****************************Checkbox*******************************

Checkbox::Checkbox(std::string in_label, int x_coord, int y_coord, void (*in_function)(Checkbox *caller), int x_width, int y_height, int in_style)
:GUIObject(x_coord, y_coord, x_width, y_height, in_style)
{
	Label = in_label;

	function = in_function;

	IsChecked = false;
	HighlightStatus = 0;
	FlagPtr = NULL;

	//Set the type
	Type = GT_CHECKBOX;
}

void Checkbox::CalculateSize()
{
	int w, h,tw,th;
	gr_get_string_size(&w, &h, (char *)Label.c_str());
	tw = w;
	th = h;

	gr_get_string_size(&w, &h, "X");
	tw += w;
	th += h + CB_TEXTCHECKDIST; //Spacing b/t 'em

	CheckCoords[0] = Coords[0];
	CheckCoords[1] = Coords[1];
	CheckCoords[2] = Coords[0] + w;
	CheckCoords[3] = Coords[1] + h;

	if(!(Style & GS_NOAUTORESIZEX))
	{
		Coords[2] = Coords[0] + tw;
	}
	if(!(Style & GS_NOAUTORESIZEY))
	{
		Coords[3] = Coords[1] + th;
	}

	if(Parent!=NULL)Parent->CalculateSize();
}

void Checkbox::DoMove(int dx, int dy)
{
	CheckCoords[0] += dx;
	CheckCoords[2] += dx;
	CheckCoords[1] += dy;
	CheckCoords[3] += dy;
}

void Checkbox::DoDraw(float frametime)
{
	if(HighlightStatus == 1)
		gr_set_color_fast(&Color_text_active);
	else if(HighlightStatus == 2)
		gr_set_color_fast(&Color_text_selected);
	else
		gr_set_color_fast(&Color_text_normal);

	draw_open_rect(CheckCoords[0], CheckCoords[1], CheckCoords[2], CheckCoords[3], false);
	if((IsChecked && FlagPtr == NULL) || (FlagPtr!= NULL && ((*FlagPtr) & Flag)))
		gr_string(CheckCoords[0], CheckCoords[1], "X", false);

	gr_set_color_fast(&Color_text_normal);
	gr_string(CheckCoords[2] + CB_TEXTCHECKDIST, CheckCoords[1], (char *)Label.c_str(), false);
}

int Checkbox::DoMouseOver(float frametime)
{
	if(Globals->GetMouseX() >= CheckCoords[0]
	&& Globals->GetMouseX() <= CheckCoords[2]
	&& Globals->GetMouseY() >= CheckCoords[1]
	&& Globals->GetMouseY() <= CheckCoords[3])
		HighlightStatus = 1;
	return OF_TRUE;
}

int Checkbox::DoMouseDown(float frametime)
{
	Globals->SetActiveObject(this);

	if(Globals->GetMouseX() >= CheckCoords[0]
	&& Globals->GetMouseX() <= CheckCoords[2]
	&& Globals->GetMouseY() >= CheckCoords[1]
	&& Globals->GetMouseY() <= CheckCoords[3])
		HighlightStatus = 2;
	return OF_TRUE;
}

int Checkbox::DoMouseUp(float frametime)
{
	if(Globals->GetMouseX() >= CheckCoords[0]
	&& Globals->GetMouseX() <= CheckCoords[2]
	&& Globals->GetMouseY() >= CheckCoords[1]
	&& Globals->GetMouseY() <= CheckCoords[3])
	{
		HighlightStatus = 1;
		if(function != NULL)
		{
			function(this);
		}
		if(FlagPtr != NULL)
		{
			if(!(*FlagPtr & Flag))
			{
				*FlagPtr |= Flag;
				IsChecked = true;
			}
			else
			{
				*FlagPtr &= ~Flag;
				IsChecked = false;
			}
		}
		else
		{
			IsChecked = !IsChecked;
		}
	}

	return OF_TRUE;
}

int Checkbox::DoMouseOut(float frametime)
{
	HighlightStatus = 0;

	return OF_TRUE;
}