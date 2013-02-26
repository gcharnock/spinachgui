
#ifndef __ORIENT_DIALOG_2__H__
#define __ORIENT_DIALOG_2__H__

#include <shared/orientation.hpp>
#include <auto/SpinachGUI.h>
#include <auto/orientdialog.h>
#include <vector>
#include <boost/optional.hpp>

typedef boost::optional<SpinXML::Orientation::Type> maybeOrientType;


//============================================================//
// OrientDialogCombo

class OrientDialog2 : public OrientDialog2Base {
public:
    //Events
    OrientDialog2(wxWindow* parent);
    void SlotRotate(Eigen::Matrix3f rotation);
    
    //Other
    void SetOrient(const SpinXML::Orientation& orient,maybeOrientType typeToKeep = maybeOrientType());
    SpinXML::Orientation GetOrient() const;

    SpinXML::EulerAngles ReadEulerEdit() const;
    Eigen::AngleAxisd  ReadAngleAxisEdit() const;
    Eigen::Quaterniond ReadQuaternionEdit() const;
    Eigen::Matrix3d    ReadDCMEdit() const;

    void WriteEulerEdit(     const SpinXML::EulerAngles& ea);
    void WriteAngleAxisEdit( const Eigen::AngleAxisd& aa);
    void WriteQuaternionEdit(const Eigen::Quaterniond& q);
    void WriteDCMEdit(       const Eigen::Matrix3d& mat);


protected:
    DECLARE_EVENT_TABLE();
    void OnEulerEdit(wxCommandEvent& e);
    void OnAngleAxisEdit(wxCommandEvent& e);
    void OnQuaternionEdit(wxCommandEvent& e);
    void OnDCMEdit(wxCommandEvent& e);
    void OnIdentity(wxCommandEvent& e);

    void OnFixMat(wxCommandEvent& e);
    void OnFixQuat(wxCommandEvent& e);
    void OnFixAA(wxCommandEvent& e);

    //Non Evetns
    void Update3D(const Eigen::Matrix3d& mat);
    void UpdateDet();
    void UpdateQNorm();
    void UpdateAANorm();
    void SetLastEdited(SpinXML::Orientation::Type type);
private:
    SpinXML::Orientation::Type mLastTypeEdited;

    std::vector<TextCtrlFocus*> mEulerAngleCtrls;
    std::vector<TextCtrlFocus*> mMatrixCtrls;
    std::vector<TextCtrlFocus*> mAngleAxisCtrls;
    std::vector<TextCtrlFocus*> mQuaternionCtrls;
};


//============================================================//
// OrientDialogCombo

class OrientDialogCombo : public wxButton {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    OrientDialogCombo(wxWindow* parent,wxWindowID id=-1);

    void SetOrient(const SpinXML::Orientation& orient);
    const SpinXML::Orientation& GetOrient() {return mOrient;}
    
    //Events
    void OnClick(wxCommandEvent& e);

    sigc::signal<void> sigChange;
protected:
    void ReadFromOrient();
    DECLARE_EVENT_TABLE();

private:
    SpinXML::Orientation mOrient;
};

#endif
