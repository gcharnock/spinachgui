#ifndef __rootFrame__
#define __rootFrame__


#include <wx/odcombo.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "spinachGUI.h"
#include "optionFrame.h"
#include "calcFrame.h"
#include "glMolDisplay.h"


/*
rootFrameBase is generated by wxFormBuilder. rootFrame inherits
rootFrameBase's properties


In order for this to compile, wxUSE_GLCANVAS must
be set to 1 in wxWidgets-2.8.9\lib\gcc_lib\mswd\wx\setup.h

*/
class rootFrame : public rootFrameBase
{
public:

	rootFrame(wxWindow* parent);


  //Event Handling functions
    void OnIdle(wxIdleEvent& e);
    void ShowOptions(wxCommandEvent& e);
    void ShowCalc(wxCommandEvent& e);
    void OnMouseMove(wxMouseEvent& e);

  //Graphics functions
	void enableGL();
	void glTick();

protected:
    void popSpinPropGrid();
    void popCouplingGrid();

    wxPropertyGrid* mSpinPropGrid;
	wxPropertyGrid* mCouplingPropGrid;
    glMolDisplay* mMolDisplay;
    DECLARE_EVENT_TABLE()

};

#endif // __rootFrame__
