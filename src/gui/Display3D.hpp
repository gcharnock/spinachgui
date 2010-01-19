
#ifndef _3DDISPLAY_H
#define _3DDISPLAY_H

#include <wx/glcanvas.h>
#include <shared/spinsys.hpp>
#include <boost/shared_ptr.hpp>

#include <gui/SpinSysManager.hpp>

#include <GL/glx.h>
#include <GL/glu.h>

#include <gui/Event.hpp>

#include <list>

using namespace boost;
using namespace SpinXML;



///A class for storing all drawing options
class SpinachDC {
public:
  GLUquadric* GetSolidQuadric() const {return mSolidQuadric;}
  GLUquadric* GetWireQuadric() const {return mWireQuadric;}
  ///If true draw only to the depth buffer
  bool depthOnly;
  GLUquadric* mSolidQuadric;
  GLUquadric* mWireQuadric;
  SpinachDC()
    : depthOnly(false),
      mSolidQuadric(gluNewQuadric()),
      mWireQuadric(gluNewQuadric()) {
    gluQuadricDrawStyle(mSolidQuadric,GLU_FILL);
    gluQuadricNormals  (mSolidQuadric,GLU_SMOOTH);
	
    gluQuadricDrawStyle(mWireQuadric,GLU_LINE);
    gluQuadricNormals  (mWireQuadric,GLU_SMOOTH);
  }
};

class SGNode {
public:
  ///Construct a dirty SGNode
  SGNode();

  ///Destruct the SGNode
  ~SGNode();

  ///Attach a child node to the SGNode
  void Attach(SGNode* node);

  ///Detatch a given node, throw if node not found.
  void Detach(SGNode* node);

  ///Mark this node as dirty, that is, needing to drewdraw its display
  ///list.
  void Dirty() {mDirty=true;}

  ///Draw, using RawDraw if needed or by calling the display list if
  ///one exists.
  void Draw(const SpinachDC& dc);

  ///
  void SetMaterial(const float material[3],bool use=true);
private:
  ///Make whatever openGL calls are needed to draw the node.
  virtual void RawDraw(const SpinachDC& dc)=0;

  bool mDirty;
  ///Stores an openGL display list for rendering the node
  int mList;
  ///Stores an openGL display list for rendering the node
  ///geomatary. Lighting and materials are not included
  int mGeomOnlyList;

  bool mUseMaterial;
  const float* mMaterial;

  std::list<SGNode*> mChildren;
  typedef std::list<SGNode*>::iterator itor;
};


class Display3D :  public wxGLCanvas, public IEventListener {
public:
  Display3D(wxWindow* parent);
  virtual ~Display3D();

  void OnPaint(wxPaintEvent& e);
  void OnMouseMove(wxMouseEvent& e);
  void OnWheel(wxMouseEvent& e);
  void OnRightClick(wxMouseEvent& e);
  void OnLeftClick(wxMouseEvent& e);
  void OnResize(wxSizeEvent& e);
  void OnDeleteSpinHover(wxCommandEvent& e);

  void SetRootSGNode(SGNode* node) {
    if(mRootNode) delete mRootNode; mRootNode=node;
  }

  DECLARE_EVENT_TABLE();

  SpinachDC* GetDC();

  void OnChange(const Event& e);
protected:
  ///Call whenever the size of the viewport changes
  

  void EnableGL();
  void ChangeViewport();

private:

  SGNode* mRootNode;

  SpinachDC mDC;

  bool mGLEnabled;
  wxGLContext* mGLContext;

  const SpinSysPtr* mSS;

  //Textures
  GLuint mTexDepth;

  //GUI State Variables
  long mHover;
  double mHoverDist;
  long mMouseX,mMouseY;
  double mZoom;

  //Quadrics
  GLUquadric* mQFilled;
  GLUquadric* mQWireframe;

  //3D Variables
  double mCamX,mCamY,mCamZ;
  float mRotationMatrix[16];
  float mXTranslate,mYTranslate;
  float mXRotate,mYRotate;
};

#endif

