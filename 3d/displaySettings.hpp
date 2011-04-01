
#ifndef __DISPLAYSETTINGS_HPP__
#define __DISPLAYSETTINGS_HPP__

#include <shared/interaction.hpp>
#include <3d/opengl.hpp>

//================================================================================//
// Whenever any of the display settings we generally have to redraw
// everything. This signal provides a signel trigger for doing that.

extern sigc::signal<void> sig3DChange;

//================================================================================//
// Wireframe settings

enum QUADRIC_TYPE {
	QUAD_WIREFRAME,
	QUAD_SOLID
};

GLUquadric* GetQuadric();
void SetQuadric(QUADRIC_TYPE type);

//================================================================================//
// RGB colour struct

struct ColourRGB {
	ColourRGB() {}
	ColourRGB(float _r,float _g, float _b) 
		: r(_r),g(_g),b(_b) {
	}
	void SetMaterial(float alpha) {
		GLfloat material[4];

		material[0] = r;
		material[1] = g;
		material[2] = b;
		material[3]=  alpha;

		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
	}
	float r;
	float g;
	float b;
};

//================================================================================//
// Should bonds be drawn?

void SetShowBonds(bool b);
bool GetShowBonds();
extern sigc::signal<void,bool> sigShowBonds;

extern ColourRGB gBondColour;


//================================================================================//
//Interaction Display Settings

void SetInterVisible(bool b     , SpinXML::Interaction::Type t);
void SetInterColour (ColourRGB c, SpinXML::Interaction::Type t);
void SetInterSize   (double s   , SpinXML::Interaction::Type t);

bool      GetInterVisible(SpinXML::Interaction::Type t);
ColourRGB GetInterColour (SpinXML::Interaction::Type t);
double    GetInterSize   (SpinXML::Interaction::Type t);

extern sigc::signal<void,SpinXML::Interaction::Type,bool>      sigInterVisible;
extern sigc::signal<void,SpinXML::Interaction::Type,ColourRGB> sigInterColour;
extern sigc::signal<void,SpinXML::Interaction::Type,double>    sigInterSize;


//================================================================================//
// Static module initalisation (private)


class __ModInit  {
public:
	__ModInit();
	~__ModInit();
};

#endif
