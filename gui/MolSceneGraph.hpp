
#ifndef _MOL_SCENE_GRAPH_H_
#define _MOL_SCENE_GRAPH_H_

#include <sigc++/sigc++.h>
#include <gui/Display3D.hpp>



class MoleculeFG : public Renderer {
public:
    MoleculeFG();
    void OnNewElectron(SpinXML::Spin* newSpin,long number);
private:
    virtual void Geometary(SpinachDC& dc) {}
};

//============================================================//

class SpinDrawer : public Renderer {
public:
    SpinDrawer();
protected:
    virtual void Geometary(const DisplaySettings& settings, PASS pass);
};


class BondDrawer : public Renderer {
public:
    BondDrawer();
protected:
    virtual void Geometary(const DisplaySettings& settings, PASS pass);
};

class LinearInterDrawer : public Renderer {
public:
    LinearInterDrawer();
protected:
    virtual void Geometary(const DisplaySettings& settings, PASS pass);
};

class FrameDrawer : public Renderer {
public:
	FrameDrawer();
protected:
	virtual void Geometary(const DisplaySettings& settings, PASS pass);
};

#endif
