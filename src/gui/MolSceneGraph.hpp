
#ifndef _MOL_SCENE_GRAPH_H_
#define _MOL_SCENE_GRAPH_H_

#include <sigc++/sigc++.h>
#include <gui/Display3D.hpp>


///Scenegraph node that draws a spin
class SpinNode : public SGNode {
public:
    SpinNode(Spin* spin);
    void OnSpinDying(Spin*) {delete this;} //Arguments are usused

    void OnSpinHover(Spin* spin);
    void OnSpinSelect(std::vector<SpinXML::Spin*> spins);
private:

    Spin* mSpin;
    virtual void RawDraw(SpinachDC& dc);
    virtual void ToPovRay(wxString& src);

    bool mHover;
    bool mSelected;
};

class InterNode : public SGNode {
public:
    InterNode(SpinXML::Spin* spin, SpinXML::Interaction::SubType st);
    void OnSpinDying(Spin*) {delete this;}
    void LoadInteractionMatrix();

    void OnNewInteraction(Interaction* inter);
private:
    SpinXML::Spin* mSpin;
    SpinXML::Interaction::SubType mType;

    void SetMatrix(const Matrix3& mat);
    virtual void RawDraw(SpinachDC& dc);
    virtual void ToPovRay(wxString& src);
    float mat[16];
};


///Scene graph node that draws a cylinder between two points
class CyclinderNode : public SGNode {
    ///Create a node that draws a cyclinder of unit radius between
    ///(0,0,0) and (0,0,1)
    CyclinderNode()
        : mRadius(1.0f),mR1(Vector3(0,0,0)),mR2(Vector3(0,0,0)) {}
    ///Create a node that draws a cyclinder of a given radius between r1
    ///and r2
    CyclinderNode(Vector3& r1, Vector3& r2, float radius)
        : mRadius(mRadius),mR1(r1),mR2(r2) {}
    ///Set the paramiters of the cyclinder
    void SetCyclinder(Vector3& r1, Vector3& r2, float radius) {
        mRadius=radius; mR1=r1; mR2=r2; Dirty();
    }
private:
    virtual void RawDraw(SpinachDC& dc);
    virtual void ToPovRay(wxString& src) {/*TODO*/}
    float mRadius;
    Vector3 mR1;
    Vector3 mR2;
};


class MoleculeNode : public SGNode {
public:
    MoleculeNode(SpinSystem* ss);
    void OnReload();
    void OnNewSpin(SpinXML::Spin* newSpin,long number);

private:
    virtual void RawDraw(SpinachDC& dc);
    SpinSystem* mSS;
    virtual void ToPovRay(wxString& src);
};

class MoleculeFG : public SGNode {
public:
    MoleculeFG(SpinSystem* ss);

    void OnNewElectron(SpinXML::Spin* newSpin,long number);

private:
    virtual void RawDraw(SpinachDC& dc) {}
    virtual void ToPovRay(wxString& src) {}
    SpinSystem* mSS;
};

#endif
