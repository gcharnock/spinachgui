
#ifndef __DISPLAYSETTINGS_HPP__
#define __DISPLAYSETTINGS_HPP__

#include <shared/interaction.hpp>
#include <3d/opengl.hpp>

#include <shared/initonce.hpp>

//================================================================================//
// Whenever any of the display settings we generally have to redraw
// everything. This signal provides a signel trigger for doing that.

extern sigc::signal<void> sig3DChange;

///A struct storing all 3D settings that are invariant over a single
///rendering pass, implying that this structure may be passed safely
///as a const reference. It would likely make sense to link settings
///here to a menu option of configuration file setting. Information
///such as the current render pass would not belong in this struct
struct DisplaySettings {
    DisplaySettings()
		: mSolidQuadric(gluNewQuadric()),
		  mWireQuadric(gluNewQuadric()) {
		gluQuadricDrawStyle(mSolidQuadric,GLU_FILL);
		gluQuadricNormals  (mWireQuadric,GLU_SMOOTH);
    }
    ~DisplaySettings() {
		gluDeleteQuadric(mSolidQuadric);
        gluDeleteQuadric(mWireQuadric);
    }

    GLUquadric* mSolidQuadric;
    GLUquadric* mWireQuadric;
    int width,height;
};

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


class __ModInit : public InitOnce<__ModInit> {
public:
	__ModInit();
};

#endif
