
#include <gui/OrientDialog2.hpp>
#include <shared/basic_math.hpp>
#include <gui/TextCtrlFocus.hpp>
#include <shared/panic.hpp>
#include <iostream>

using namespace Eigen;
using namespace sigc;
using namespace std;
using namespace SpinXML;

OrientDialog2::OrientDialog2(wxWindow* parent)
    : OrientDialog2Base(parent,wxID_ANY,wxT("Orientation"),wxDefaultPosition, wxSize( 600,350 )) {

    SetLastEdited(Orientation::EULER);
    mOrientDisplay3D->sigRotate.connect(mem_fun(this,&OrientDialog2::SlotRotate));

    SetOrient(Orientation(EulerAngles(0,0,0)));
}

void OrientDialog2::SlotRotate(Eigen::Matrix3f rotation) {
    Matrix3d rotationd = Matrix3f2Matrix3d(rotation);
    Orientation o(rotationd);
    SetOrient(o);
}



void OrientDialog2::SetLastEdited(Orientation::Type type) {
    mLastTypeEdited = type;

    mEulerAnglesSizer->GetStaticBox()->SetLabel(type==Orientation::EULER      ? wxT("Euler Angles*") : wxT("Euler Angles"));
    mDCMSizer        ->GetStaticBox()->SetLabel(type==Orientation::DCM        ? wxT("Matrix*")       : wxT("Matrix"));
    mAngleAxisTimer  ->GetStaticBox()->SetLabel(type==Orientation::ANGLE_AXIS ? wxT("Angle Axis*")   : wxT("Angle Axis"));
    mQuaternionSizer ->GetStaticBox()->SetLabel(type==Orientation::QUATERNION ? wxT("Quaternion*")   : wxT("Quaternion"));
}

void OrientDialog2::SetOrient(const Orientation& orient,maybeOrientType typeToKeep) {
    EulerAngles ea  = orient.GetAsEuler();
    Matrix3d    mat = orient.GetAsDCM();
    AngleAxisd  aa  = orient.GetAsAngleAxis();
    Quaterniond q   = orient.GetAsQuaternion();

    if(!typeToKeep || typeToKeep.get() != Orientation::EULER) {
        mAlpha->ChangeValue(wxString() << ea.alpha /PI*180);
        mBeta ->ChangeValue(wxString() << ea.beta  /PI*180);
        mGamma->ChangeValue(wxString() << ea.gamma /PI*180);
    }
    if(!typeToKeep || typeToKeep.get() != Orientation::DCM) {
        m00->ChangeValue(wxString() << mat(0,0));
        m01->ChangeValue(wxString() << mat(0,1));
        m02->ChangeValue(wxString() << mat(0,2));

        m10->ChangeValue(wxString() << mat(1,0));
        m11->ChangeValue(wxString() << mat(1,1));
        m12->ChangeValue(wxString() << mat(1,2));

        m20->ChangeValue(wxString() << mat(2,0));
        m21->ChangeValue(wxString() << mat(2,1));
        m22->ChangeValue(wxString() << mat(2,2));
    }
    if(!typeToKeep || typeToKeep.get() != Orientation::ANGLE_AXIS) {
        mAngle->ChangeValue(wxString() << aa.angle());
        mX    ->ChangeValue(wxString() << aa.axis().x());
        mY    ->ChangeValue(wxString() << aa.axis().y());
        mZ    ->ChangeValue(wxString() << aa.axis().z());
    }
    if(!typeToKeep || typeToKeep.get() != Orientation::QUATERNION) {
        mRe->ChangeValue(wxString() << q.w());
        mI ->ChangeValue(wxString() << q.x());
        mJ ->ChangeValue(wxString() << q.y());
        mK ->ChangeValue(wxString() << q.z());
    }
    Matrix3f mat3f = Matrix3d2Matrix3f(mat);
    mOrientDisplay3D->GetCamera()->SetRotation(mat3f);
    mOrientDisplay3D->Refresh();
    UpdateDet();
}

Orientation OrientDialog2::GetOrient() const {
    switch(mLastTypeEdited) {
    case Orientation::EULER:
        return Orientation(ReadEulerEdit());
        break;
    case Orientation::ANGLE_AXIS:
        return Orientation(ReadAngleAxisEdit());
        break;
    case Orientation::QUATERNION:
        return Orientation(ReadQuaternionEdit());
        break;
    case Orientation::DCM:
        return Orientation(ReadDCMEdit());
        break;
    default:
        PANIC();
        return Orientation();
    }
}


EulerAngles OrientDialog2::ReadEulerEdit() const {
    double alpha,beta,gamma;
    mAlpha->GetValue().ToDouble(&alpha);
    mBeta ->GetValue().ToDouble(&beta);
    mGamma->GetValue().ToDouble(&gamma);
    return EulerAngles(alpha*PI/180,beta*PI/180,gamma*PI/180);
}

AngleAxisd OrientDialog2::ReadAngleAxisEdit() const {
    double angle,x,y,z;
    mAngle->GetValue().ToDouble(&angle);
    mX->GetValue().ToDouble(&x);
    mY->GetValue().ToDouble(&y);
    mZ->GetValue().ToDouble(&z);
    return AngleAxisd(angle,Vector3d(x,y,z));
}
Quaterniond OrientDialog2::ReadQuaternionEdit() const {
    double re,i,j,k;
    mRe->GetValue().ToDouble(&re);
    mI->GetValue().ToDouble(&i);
    mJ->GetValue().ToDouble(&j);
    mK->GetValue().ToDouble(&k);
    return Quaterniond(re,i,j,k);
}

Matrix3d OrientDialog2::ReadDCMEdit() const {
    double
        mat00,mat10,mat20,
        mat01,mat11,mat21,
        mat02,mat12,mat22;

    m00->GetValue().ToDouble(&mat00);
    m10->GetValue().ToDouble(&mat10);
    m20->GetValue().ToDouble(&mat20);
              
    m01->GetValue().ToDouble(&mat01);
    m11->GetValue().ToDouble(&mat11);
    m21->GetValue().ToDouble(&mat21);
              
    m02->GetValue().ToDouble(&mat02);
    m12->GetValue().ToDouble(&mat12);
    m22->GetValue().ToDouble(&mat22);

    return MakeMatrix3d(mat00,mat10,mat20,
                        mat01,mat11,mat21,
                        mat02,mat12,mat22);
}

void OrientDialog2::OnEulerEdit(wxCommandEvent& e) {
    SetLastEdited(Orientation::EULER);
    SetOrient(GetOrient(),Orientation::EULER);
}
void OrientDialog2::OnAngleAxisEdit(wxCommandEvent& e) {
    SetLastEdited(Orientation::ANGLE_AXIS);
    SetOrient(GetOrient(),Orientation::ANGLE_AXIS);
}
void OrientDialog2::OnQuaternionEdit(wxCommandEvent& e) {
    SetLastEdited(Orientation::QUATERNION);
    SetOrient(GetOrient(),Orientation::QUATERNION);
}
void OrientDialog2::OnDCMEdit(wxCommandEvent& e) {
    SetLastEdited(Orientation::DCM);
    SetOrient(GetOrient(),Orientation::DCM);
}

void OrientDialog2::OnIdentity(wxCommandEvent& e) {
    SetOrient(EulerAngles(0,0,0));
}

void OrientDialog2::UpdateDet() {
    //We should always make sure the determinate is for the matrix
    //being displayed
    Matrix3d mat = this->ReadDCMEdit();

    double determinant = mat.determinant();
    mTextDet->SetLabel(wxString() << wxT("Det: ") << determinant);
}

BEGIN_EVENT_TABLE(OrientDialog2,wxDialog)
EVT_TEXT(ID_EULER    ,OrientDialog2::OnEulerEdit)
EVT_TEXT(ID_ANGLEAXIS,OrientDialog2::OnAngleAxisEdit)
EVT_TEXT(ID_QUAT     ,OrientDialog2::OnQuaternionEdit)
EVT_TEXT(ID_MATRIX   ,OrientDialog2::OnDCMEdit)

EVT_BUTTON(ID_IDENTITY,OrientDialog2::OnIdentity)

END_EVENT_TABLE()


//================================================================================//
// OrientEditDialogCombo

OrientDialogCombo::OrientDialogCombo(wxWindow* parent,wxWindowID id) 
: wxButton(parent,id,wxT("")) {
}

void OrientDialogCombo::SetOrient(const Orientation& orient) {
    mOrient=orient;
    ReadFromOrient();
}

void OrientDialogCombo::ReadFromOrient() {
    switch(mOrient.GetType()) {
    case Orientation::EULER:
	SetLabel(wxString(wxT("Euler Angles (ZYZ)")));
	break;
    case Orientation::ANGLE_AXIS:
	SetLabel(wxString(wxT("Angle Axis")));
	break;
    case Orientation::QUATERNION:
	SetLabel(wxString(wxT("Quaternion")));
	break;
    case Orientation::DCM:
	SetLabel(wxString(wxT("DCM")));
	break;
    };

}

void OrientDialogCombo::OnClick(wxCommandEvent& e) {
    OrientDialog2* dlg=new OrientDialog2(this);
    dlg->SetOrient(mOrient);

    if (dlg->ShowModal() == wxID_OK) {
	mOrient = dlg->GetOrient();
	ReadFromOrient();
    }
    dlg->Destroy();
    SetFocus();
    sigChange();
}


BEGIN_EVENT_TABLE(OrientDialogCombo,wxButton)

EVT_BUTTON(wxID_ANY,OrientDialogCombo::OnClick)

END_EVENT_TABLE()
