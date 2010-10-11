#if 0
#include <shared/spinsys.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <shared/nuclear_data.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

using namespace std;
using namespace SpinXML;
using namespace sigc;
using namespace boost;


//==============================================================================//
// SpinSystem

SpinSystem::SpinSystem() : mIgnoreSpinKill(NULL) {
}


SpinSystem::SpinSystem(const SpinSystem& system)  {
    long spinCount=system.mSpins.size();

    mSpins.resize(spinCount);

    for(long i=0;i<spinCount;i++) {
        mSpins[i]=new Spin(*system.mSpins[i]);
    }
}


SpinSystem::~SpinSystem() {
    Clear();
    sigDying();
}

void SpinSystem::Clear() {
    //Currently we need to delete the interactions Before the spins as
    //the spins have interactions as their children. 

    for(unsigned long i=0;i<mSpins.size();i++) {
        mIgnoreSpinKill=mSpins[i];
        delete mSpins[i];
    }
    mSpins.resize(0);

    sigReloaded();
}

void SpinSystem::Dump() const {
    cout << "Dumping SpinSystem:" << endl;
    cout << " Spin count=" << mSpins.size() << endl;
    cout << endl;
}

long SpinSystem::GetSpinCount() const {
    return mSpins.size();
}

Spin* SpinSystem::GetSpin(long n) const {
    return mSpins[n];
}

long SpinSystem::GetSpinNumber(Spin* spin) const {
    for(unsigned long i=0;i<mSpins.size();i++) {
        if(mSpins[i]==spin) {
            return i;
        }
    }
    return -1;
}

vector<Spin*> SpinSystem::GetNearbySpins(Vector3l pos,length distance,Spin* Ignore) {
    std::vector<Spin*> result;
    length2 dist2=distance*distance;
    length x1=pos.GetX();
    length y1=pos.GetY();
    length z1=pos.GetZ();

    long spinCount=mSpins.size();

    long skipped;
    for(skipped=0;mSpins[skipped]!=Ignore && skipped<spinCount;skipped++){}


    for(long i=skipped+1;i<spinCount;i++) {
        length x2,y2,z2;
        mSpins[i]->GetCoordinates(&x2,&y2,&z2);
        length deltaX=(x1-x2);
        length deltaY=(y1-y2);
        length deltaZ=(z1-z2);
	length2 deltaR=deltaX*deltaX+deltaY*deltaY+deltaZ*deltaZ;
        if(deltaR < dist2) {
            result.push_back(mSpins[i]);
        }
    }
    return result;
}
const vector<Spin*>& SpinSystem::GetSpins() const {
    return mSpins;
}

void SpinSystem::InsertSpin(Spin* _Spin,long Position) {

    if(Position==END) {
        mSpins.push_back(_Spin);
        sigNewSpin(_Spin,mSpins.size()-1);
    } else {
        mSpins.insert(mSpins.begin()+Position,_Spin);
        sigNewSpin(_Spin,Position);
    }

    _Spin->sigDying.connect(mem_fun(*this,&SpinSystem::OnSpinDeleted));
}


void SpinSystem::RemoveSpin(long Position) {
    mSpins.erase(mSpins.begin()+Position);
}

void SpinSystem::RemoveSpin(Spin* _Spin) {
    if(_Spin==mIgnoreSpinKill) {
        mIgnoreSpinKill=NULL;
        return;
    }
    for(unsigned long i=0;i<mSpins.size();i++) {
        if(mSpins[i]==_Spin) {
            mSpins.erase(mSpins.begin()+i);
            return;
        }
    }
}


void SpinSystem::LoadFromFile(const char* filename,ISpinSystemLoader* loader) {
    try {
        loader->LoadFile(this,filename);
    } catch(const runtime_error& e) {
        sigReloaded();
        throw e;
    }
    sigReloaded();
    return;
}

void SpinSystem::SaveToFile(const char* filename,ISpinSystemLoader* saver) const 
{
    saver->SaveFile(this,filename);
    return;
}

void SpinSystem::CalcNuclearDipoleDipole() {
    for(unsigned long i=0;i<mSpins.size();i++) {
        Spin* spin1=mSpins[i];
        long element1=spin1->GetElement();
        long isotope1=spin1->GetIsotopes()[0];
        double g1=getGyromagneticRatio(element1,isotope1);
        if(abs(g1)<1e-10) continue;

        for(unsigned long j=i+1;j<mSpins.size();j++) {
            Spin* spin2=mSpins[j]; 

            //A unit vector pointing from spin1 tp spin2
            length rx,ry,rz;
            Vector3l r_1_2=spin2->GetPosition()-spin1->GetPosition();
            r_1_2.GetCoordinates(&rx,&ry,&rz);

            long element2=spin2->GetElement();
            long isotope2=spin2->GetIsotopes()[0];
            double g2=getGyromagneticRatio(element2,isotope2);
            if(abs(g2)<1e-10) continue;
            
            //cout << "isotope1=" << isotope1 << "  isotope2="<< isotope2 << endl;
            //cout << "g1="<< g1 << "  g2="<< g2 << endl;

            length r=(r_1_2.GetLength());
            length2 r2=r*r;
            length3 r3=r2*r;
            length5 r5=r2*r3;

            Matrix3l2 dipole(length2(r2-length2(3.0*rx*rx)),
			     length2(3.0*rx*ry),
			     length2(3.0*rx*rz),

		             length2(3.0*rx*ry),
			     length2(r2-length2(3.0*ry*ry)),
			     length2(3.0*ry*rz),
			     
			     length2(3.0*rx*rz),
			     length2(3.0*ry*rz),
			     length2(r2-length2(3.0*rz*rz)));
            static const double four_pi=12.5663706;
#warning "Still need to check this calculation is producing sensible numbers"
            double coeff=(mu0*hbar*g1*g2/(r5.si*four_pi));

	    dreal<double,_mass_per_time2>
		dcoeff(coeff);
            Matrix3e dipole_inter=dipole * dcoeff;
            Interaction* inter=new Interaction;
            inter->SetMatrix(dipole_inter);
            inter->SetSubType(Interaction::ST_DIPOLAR,spin1,spin2);
        }
    }
}





//==============================================================================//
// Interaction

Interaction::Interaction() 
    : mSubType(ST_ANY),mSpin1(NULL),mSpin2(NULL) {
}

Interaction::Interaction(const Interaction& inter,SpinSystem* system) :
    mData(mData),
    mSubType(inter.mSubType),
    mSpin1(inter.mSpin1),
    mSpin2(inter.mSpin2)  {
}

Interaction::~Interaction() {
    sigDying(this);
}

class thisVisitor : public static_visitor<> {
public:
    void operator()(const energy& dat) const {
	cout << "    Type=Scalar" <<  endl;
	cout << "    Value=" << dat[MHz] << "MHz" << endl;
    }
    void operator()(const Matrix3e& dat) const {
	cout << "    Type=Matrix" <<  endl;
	cout << "    Value=" << endl;
    }
    void operator()(const eigenvalues_t& dat) const {
	cout << "    Type=Eigenvalues" <<  endl;
	cout << "    Value=(XX=" 
	     << dat.xx[MHz] << "MHz ,YY="
	     << dat.yy[MHz] << "MHz ,ZZ="
	     << dat.zz[MHz] << "MHz)"
	     << endl;
	cout << "    Orient=";
    }
    void operator()(const ax_rhom_t& dat) const {
	cout << "    Type=Axiality Rhombicity" <<  endl;
	cout << "    Value=" << endl; 
	cout << "    Orient=";
	cout << dat.mOrient.ToString() << endl;
    }
    void operator()(const span_skew_t& dat) const {
	cout << "    Type=Span-Skew" <<  endl;
	cout << "    Value=" << endl;
	cout << "    Orient=";
	cout << dat.mOrient.ToString() << endl;
    }
};
void Interaction::Dump() const {
    cout << "  Dumping an Interaction" << endl;
    apply_visitor(thisVisitor(),mData);
}

const char* Interaction::GetTypeName(Type t) {
    switch(t) {
    case UNDEFINED:       return "Undefined";
    case SCALAR:          return "Scalar";      
    case MATRIX:          return "Matrix";     
    case EIGENVALUES:     return "Eigenvalues";
    case AXRHOM:          return "Axiality-Rhombicty";
    case SPANSKEW:        return "Span-Skew";
    default:
        ostringstream stream;
        stream << "Unknow type submited to Interaction::GetTypeName() (t=";
        stream << t << ")" << endl;
        throw std::runtime_error(stream.str());
        return "Error in Interaction::GetTypeName()";
    };         
};


const char* Interaction::GetSubTypeName(SubType st) {
    switch(st) {
    case ST_ANY:         return "Any";
    case ST_EPR:         return "EPR";
    case ST_NMR:         return "NMR";
    case ST_HFC:         return "HFC";
    case ST_G_TENSER:    return "G Tensor";
    case ST_ZFS: 	       return "Zero Field Splitting";
    case ST_EXCHANGE:    return "Exchange";
    case ST_QUADRUPOLAR: return "Quadrupolar";
    case ST_DIPOLAR:     return "Dipolar";
    case ST_SHIELDING:   return "Shielding";
    case ST_SCALAR:      return "Scalar";
    case ST_CUSTOM_LINEAR:     return "Custom (Linear)";
    case ST_CUSTOM_BILINEAR:   return "Custom (Bilinear)";
    case ST_CUSTOM_QUADRATIC:  return "Custom (Quadratic)";
    default:
        ostringstream stream;
        stream << "Unknow subtype submited to Interaction::GetSubTypeName() (st=";
        stream << st << ")" << endl;
        throw std::runtime_error(stream.str());
        return "Error in Interaction::GetSubTypeName()";
    }
}

Interaction::Type Interaction::GetType() const {
    if(get<energy>(&mData)!=NULL) {
        return SCALAR;
    } else if(get<Matrix3e>(&mData)!=NULL) {
        return MATRIX;
    } else if(get<eigenvalues_t>(&mData)!=NULL) {
        return EIGENVALUES;
    } else if(get<ax_rhom_t>(&mData)!=NULL) {
        return AXRHOM;
    } else  {
        return SPANSKEW;
    }
}



void Interaction::GetScalar(energy* Scalar) const {
    *Scalar=get<energy>(mData);
}

void Interaction::GetMatrix(Matrix3e* OutMatrix) const {
    *OutMatrix=get<Matrix3e>(mData);
    return;
}

void Interaction::GetEigenvalues(energy* XX,energy* YY, energy* ZZ, Orientation* OrientOut) const {
    *XX=get<eigenvalues_t>(mData).xx;
    *YY=get<eigenvalues_t>(mData).yy;
    *ZZ=get<eigenvalues_t>(mData).zz;
    *OrientOut=get<eigenvalues_t>(mData).mOrient;
    return;
}

void Interaction::GetAxRhom(energy* iso,energy* ax, energy* rh, Orientation* OrientOut) const {
    *iso=get<ax_rhom_t>(mData).iso;
    *ax= get<ax_rhom_t>(mData).ax;
    *rh= get<ax_rhom_t>(mData).rh;
    *OrientOut=get<ax_rhom_t>(mData).mOrient;
    return;
}

void Interaction::GetSpanSkew(energy* iso,energy* Span, double* Skew, Orientation* OrientOut) const {
    *iso= get<span_skew_t> (mData).iso;
    *Span=get<span_skew_t>(mData).span;
    *Skew=get<span_skew_t>(mData).skew;
    *OrientOut=get<span_skew_t>(mData).mOrient;
    return;
}

void Interaction::SetScalar(energy Scalar) {
    sigChange();
    mData=Scalar;
}

void Interaction::SetMatrix(const Matrix3e& Matrix) {
    sigChange();
    mData=Matrix3e(Matrix);
}

void Interaction::SetEigenvalues(energy XX,energy YY, energy ZZ, const Orientation& Orient) {
    sigChange();
    mData=eigenvalues_t(XX,YY,ZZ,Orient);
    return;
}

void Interaction::SetAxRhom(energy iso,energy ax, energy rh, const Orientation& Orient) {
    sigChange();
    mData=ax_rhom_t(iso,ax,rh,Orient);
    return;
}

void Interaction::SetSpanSkew(energy iso,energy Span, double Skew, const Orientation& Orient) {
    sigChange();
    mData=span_skew_t(iso,Span,Skew,Orient);
    return;
}


Interaction::SubType Interaction::GetSubType() const {
    return mSubType;
}

void Interaction::SetSubType(SubType st,Spin* spin1,Spin* spin2) {
    mSubType=st;
    //true if mSpin1 is not mentioned and thus will not be kept.
    bool loseSpin1=mSpin1!=spin1 && mSpin1!=spin2;
    //ture if mSpin2 is not mentioned and thus will not be kept.
    bool loseSpin2=mSpin2!=spin1 && mSpin2!=spin2;

    if((loseSpin1 && loseSpin2) || (mSpin1==mSpin2)) {
        //Easyest case, we're replacing both spins
        sigRemoveSpin(this,mSpin1);
        mConnect1.disconnect();
        mDyingConnect1.disconnect();
        sigRemoveSpin(this,mSpin2);
        mConnect2.disconnect();
        mDyingConnect2.disconnect();
        mSpin1=spin1;
        if(spin1) {
            mSpin1->InsertInteraction(this);
            mConnect1=sigRemoveSpin.connect(mem_fun(mSpin1,&Spin::OnRemoveInteraction));
            mDyingConnect1=mSpin1->sigDying.connect(mem_fun(this,&Interaction::OnSpinDying));
        }
        mSpin2=spin2;
        if(spin2){
            mSpin2->InsertInteraction(this);
            mConnect2=sigRemoveSpin.connect(mem_fun(mSpin2,&Spin::OnRemoveInteraction));
            mDyingConnect2=mSpin2->sigDying.connect(mem_fun(this,&Interaction::OnSpinDying));
        }
    } else  if(loseSpin1) {
        sigRemoveSpin(this,mSpin1);
        mConnect2.disconnect();
        mDyingConnect2.disconnect();
        //We're keeping mSpin2. But is it spin1 or spin2 that replaces mSpin1
        if(spin1==mSpin2) {
            //Replace with spin2
            mSpin1=spin2;
        } else {
            //Replace with spin1
            mSpin1=spin1;
        }
        if(mSpin1!=NULL){
            mSpin1->InsertInteraction(this);
            mConnect1=sigRemoveSpin.connect(mem_fun(mSpin1,&Spin::OnRemoveInteraction));
            mDyingConnect1=mSpin1->sigDying.connect(mem_fun(this,&Interaction::OnSpinDying));
        }
    } else if (loseSpin2) {
        //We're keeping mSpin1. But is it spin1 or spin2 that replaces mSpin2
        sigRemoveSpin(this,mSpin2);
        mConnect1.disconnect();
        mDyingConnect1.disconnect();
        if(spin1==mSpin1) {
            //Replace with spin2
            mSpin2=spin2;
        } else {
            //Replace with spin1
            mSpin2=spin1;
        }
        if(mSpin2!=NULL) {
            mSpin2->InsertInteraction(this);
            mConnect2=sigRemoveSpin.connect(mem_fun(mSpin2,&Spin::OnRemoveInteraction));
            mDyingConnect2=mSpin2->sigDying.connect(mem_fun(this,&Interaction::OnSpinDying));
        }
    }
    sigChange();
}

Interaction::Form Interaction::GetFormFromSubType(SubType st) {
    switch(st) {
    case ST_CUSTOM_LINEAR:
    case ST_SHIELDING:
    case ST_G_TENSER:
        return LINEAR;

    case ST_CUSTOM_BILINEAR:
    case ST_SCALAR:
    case ST_DIPOLAR:
    case ST_HFC:
    case ST_EXCHANGE:
        return BILINEAR;

    case ST_ZFS: 
    case ST_QUADRUPOLAR:
    case ST_CUSTOM_QUADRATIC:
        return QUADRATIC;
    case ST_ANY:
    case ST_NMR:
    case ST_EPR:
        throw logic_error("Can't pass ST_ANY,ST_NMR,ST_EPR to GetFromFromSubType");
    default:
        throw logic_error("Unknown type in GetFromFromSubType");
    }

}

bool Interaction::IsSubType(SubType t) const {
    if(t==ST_ANY) {
        return true;
    }
    if(t==ST_EPR) {
        switch(mSubType) {
        case ST_HFC:
        case ST_G_TENSER:
        case ST_ZFS: 
        case ST_EXCHANGE:
        case ST_QUADRUPOLAR:
        case ST_DIPOLAR:
        case ST_CUSTOM_LINEAR:
        case ST_CUSTOM_BILINEAR:
        case ST_CUSTOM_QUADRATIC:
            return true;
        default:
            return false;
        }
    }
    if(t==ST_NMR) {
        switch(mSubType) {
        case ST_SHIELDING:
        case ST_SCALAR:
        case ST_QUADRUPOLAR:
        case ST_DIPOLAR:
        case ST_CUSTOM_LINEAR:
        case ST_CUSTOM_BILINEAR:
        case ST_CUSTOM_QUADRATIC:
            return true;
        default:
            return false;
        }
    }
    if(t==mSubType) {
        return true;
    }
    return false;
}


bool Interaction::GetIsLinear()     {
    return 
        mSubType==ST_EXCHANGE    ||
        mSubType==ST_SHIELDING   ||
        mSubType==ST_G_TENSER    ||
        mSubType==ST_CUSTOM_LINEAR;
}

bool Interaction::GetIsBilinear()  {
    return
        mSubType==ST_HFC            ||
        mSubType==ST_DIPOLAR        ||
        mSubType==ST_EXCHANGE       ||
        mSubType==ST_SCALAR         ||
        mSubType==ST_CUSTOM_BILINEAR;

}
bool Interaction::GetIsQuadratic() {
    return 
        mSubType==ST_ZFS             ||
        mSubType==ST_QUADRUPOLAR     ||
        mSubType==ST_CUSTOM_QUADRATIC;

}

struct getAsScalarVisitor : public static_visitor<energy> {
    energy operator()(const energy& dat) const {
        return dat;
    }
    energy operator()(const Matrix3e& dat) const {
	return dat.Trace()/3.0;
    }
    energy operator()(const eigenvalues_t& dat) const {
	return dat.xx+dat.yy+dat.zz;
    }
    energy operator()(const ax_rhom_t& dat) const {
	return dat.iso*3.0;
    }
    energy operator()(const span_skew_t& dat) const {
	return dat.iso*3.0;
    }
};
void Interaction::ToScalar() {
    mData=apply_visitor(getAsScalarVisitor(),mData);
}
Interaction Interaction::AsScalar() const {
    return apply_visitor(getAsScalarVisitor(),mData);
}

struct getAsMatrixVisitor : public static_visitor<Matrix3e> {
    Matrix3e operator()(const energy& dat) const {
        //Return the identity multipled by the scalar
        return Matrix3e(dat,       0.0*Joules,0.0*Joules,
                        0.0*Joules,dat,       0.0*Joules,
                        0.0*Joules,0.0*Joules,dat        );
    }
    Matrix3e operator()(const Matrix3e& dat) const {
        //Just return the matrix
        return dat;
    }
    Matrix3e operator()(const eigenvalues_t& dat) const {
        //Convert to a matrix
	return fromev(dat.xx,dat.yy,dat.zz,dat.mOrient);
    }
    Matrix3e operator()(const ax_rhom_t& dat) const {
        eigenvalues_t edat = dat.AsEigenvalues();
	return fromev(edat.xx,edat.yy,edat.zz,dat.mOrient);
    }
    Matrix3e operator()(const span_skew_t& dat) const {
        eigenvalues_t edat = dat.AsEigenvalues();
	return fromev(edat.xx,edat.yy,edat.zz,dat.mOrient);
    }
private:
    Matrix3e fromev(const energy xx,const energy yy,const energy zz,const Orientation& o) const {
        cout << "Converting " << o.ToString() << endl;
        Matrix3 intMatrix=o.GetAsMatrix();

        cout << "Rotation Matrix determinate:" << intMatrix.det().si << endl;;

	const static energy zero=0.0*Joules;
	Matrix3e in_eigen_frame(xx,  zero,zero, 
				zero,yy  ,zero, 
				zero,zero,zz   );
        //Undo the eigensystem decomposition
	Matrix3e result=intMatrix*in_eigen_frame*intMatrix.Inverted();
        return result;
    }
};
void Interaction::ToMatrix() {
    mData=apply_visitor(getAsMatrixVisitor(),mData);
}
Interaction Interaction::AsMatrix() const {
    return Interaction(apply_visitor(getAsMatrixVisitor(),mData));
}

struct getAsEigenvaluesVisitor : public static_visitor<eigenvalues_t> {
    eigenvalues_t operator()(const energy& dat) const {
        //Return the identity multipled by the scalar
        return eigenvalues_t(dat,dat,dat,Orientation(Quaternion(1,0,0,0)));
    }
    eigenvalues_t operator()(const Matrix3e& dat) const {
        energy e1,e2,e3;//Eigenvalues
        Vector3 v1,v2,v3;//Eigenvectors
        dat.eig(e1,e2,e3,
                v1,v2,v3);
        return eigenvalues_t(e1,e2,e3,Orientation(Matrix3(v1.GetX().si,v2.GetX().si,v3.GetX().si,
                                                          v1.GetY().si,v2.GetY().si,v3.GetY().si,
                                                          v1.GetZ().si,v2.GetZ().si,v3.GetZ().si)));
    }
    eigenvalues_t operator()(const eigenvalues_t& dat) const {
	return dat;
    }
    eigenvalues_t operator()(const ax_rhom_t& dat) const {
	return dat.AsEigenvalues();
    }
    eigenvalues_t operator()(const span_skew_t& dat) const {
	return dat.AsEigenvalues();
    }
};
void Interaction::ToEigenvalues() {
    mData=apply_visitor(getAsEigenvaluesVisitor(),mData);
}
Interaction Interaction::AsEigenvalues() const {
    return Interaction(apply_visitor(getAsEigenvaluesVisitor(),mData));
}

struct getAsAxRhomVisitor : public static_visitor<ax_rhom_t> {
    ax_rhom_t operator()(const energy& dat) const {
        return ax_rhom_t(0.0*Joules,0.0*Joules,dat,Quaternion(1,0,0,0));
    }
    ax_rhom_t operator()(const Matrix3e& dat) const {
        cerr << "Skipping a hard conversion" << endl;
        energy zero=0.0*Joules;
        return ax_rhom_t(zero,zero,zero,Orientation(Quaternion(1,0,0,0)));
    }
    ax_rhom_t operator()(const eigenvalues_t& dat) const {
        cout << "Enterning eigenvalues->ax_rhom visitor" << endl;
	return dat.AsAxRhom();
    }
    ax_rhom_t operator()(const ax_rhom_t& dat) const {
	return dat;
    }
    ax_rhom_t operator()(const span_skew_t& dat) const {
	return dat.AsAxRhom();
    }
};
void Interaction::ToAxRhom() {
    cout << "ToAxRhom" << endl;
    mData=apply_visitor(getAsAxRhomVisitor(),mData);
}
Interaction Interaction::AsAxRhom() const {
    cout << "AsAxRhom" << endl;
    return Interaction(apply_visitor(getAsAxRhomVisitor(),mData));
}

struct getAsSpanSkewVisitor : public static_visitor<span_skew_t> {
    span_skew_t operator()(const energy& dat) const {
        return span_skew_t(dat,0.0*Joules,0.0,Quaternion(1,0,0,0));
    }
    span_skew_t operator()(const Matrix3e& dat) const {
        cerr << "Skipping a hard conversion" << endl;
        energy zero=0.0*Joules;
        return span_skew_t(zero,zero,0,Orientation(Quaternion(1,0,0,0)));
    }
    span_skew_t operator()(const eigenvalues_t& dat) const {
 	return dat.AsSpanSkew();
    }
    span_skew_t operator()(const ax_rhom_t& dat) const {
	return dat.AsSpanSkew();
    }
    span_skew_t operator()(const span_skew_t& dat) const {
	return dat;
    }
};
void Interaction::ToSpanSkew() {
    mData=apply_visitor(getAsSpanSkewVisitor(),mData);
}
Interaction Interaction::AsSpanSkew() const {
    return Interaction(apply_visitor(getAsSpanSkewVisitor(),mData));
}
#endif
