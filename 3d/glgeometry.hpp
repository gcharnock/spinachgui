
#ifndef __GL_GEOM_HPP__
#define __GL_GEOM_HPP__

#include <3d/displaySettings.hpp>

#include <3d/glmode.hpp>
#include <3d/opengl.hpp>



//Maybe this only needs to be an enum. We'll see
enum PASS {
	SOLID,
	TRANSLUCENT,
	PICKING
};





///An abstract class who's job is to visualise part of a spin system
///such as spins, linear interactions or bonds.
class Renderer {
public:
    Renderer();
    virtual ~Renderer();
    void DrawWith(GLMode& mode) const;
    void Draw() const;
protected:
    virtual void Geometary() const = 0;
private:
};


///Keeps a collection of renderers and manages gl state common to a
///scene, such as camera position, global rotation and lighting.
class Scene : public Renderer {
public:
    typedef std::vector<Renderer*> TRenderVec;
    typedef std::vector<Renderer*>::const_iterator TRenderIt;
    Scene(const TRenderVec& renderers);
    ~Scene();
protected:
    void Geometary() const;
private:
    TRenderVec mRenderers;
};



class MoleculeFG : public Renderer {
public:
    MoleculeFG();
    void OnNewElectron(SpinXML::Spin* newSpin,long number);
protected:
    virtual void Geometary() const {}
};

//============================================================//

class SpinDrawer : public Renderer {
public:
    SpinDrawer();
protected:
    virtual void Geometary() const;
};


class BondDrawer : public Renderer {
public:
    BondDrawer();
protected:
    virtual void Geometary() const;
};

class MonoInterDrawer : public Renderer {
public:
    MonoInterDrawer();
protected:
    virtual void Geometary() const;
};

class BilinearInterDrawer : public Renderer {
public:
    BilinearInterDrawer();
protected:
    virtual void Geometary() const;
};

class FrameDrawer : public Renderer {
public:
	FrameDrawer();
protected:
	virtual void Geometary() const;
};

//================================================================================//

class SpinSysScene : public Renderer {
public:
    SpinSysScene();
protected:
	virtual void Geometary() const;
private:
	SpinDrawer          mSpinDrawer;
	BondDrawer          mBondDrawer;
	FrameDrawer         mFrameDrawer;
};

class InteractionScene : public Renderer {
public:
	InteractionScene();
protected:
	virtual void Geometary() const;
private:
	MonoInterDrawer     mMonoDrawer;
	BilinearInterDrawer mBinaryDrawer;

};

#endif
