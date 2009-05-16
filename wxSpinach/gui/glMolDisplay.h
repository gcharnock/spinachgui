
#ifndef GL_MOL_DISPLAY
#define GL_MOL_DISPLAY

#include "../shared/spin.h"

#include <wx/glcanvas.h>
#include <wx/timer.h>

class rootFrame;

class glMolDisplay : public wxGLCanvas {
public:
    glMolDisplay(rootFrame* parent);
    void glTick();
    void enableGL();;  //Resolve the cyclic dependency by replying on the fact that all pointers are alike

protected:
    rootFrame* mParentFrame;

    void OnMouseMove(wxMouseEvent& e);
    void OnResize(wxSizeEvent& e);
    void DoReize();
    void OnWheel(wxMouseEvent& e);
    void OnTimer(wxTimerEvent& e);
    DECLARE_EVENT_TABLE()

    GLuint list;
    long mousex;
    long mousey;

  //A place to store the window size between OnResize and the
  //window being redrawn.
    int width;
    int height;

  //The current zoom level, multiplys the screen width and screen height in glOrth
    double mZoom;

    double camX;
    double camY;
    double camZ;

    double xRotate;
    double yRotate;

    float rotationMatrix[16];
    wxTimer mTimer;
	wxGLContext* mpGLContext;

	bool mInitalised;
};

#endif
