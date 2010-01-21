
#ifndef SPINSYS_H;
#define SPINSYS_H;

//#undef SPINXML_EVENTS

#include <vector>
#include <string>
#include <string.h>
#include <stdexcept>
#include <shared/mathtypes.hpp>
#include <sigc++/sigc++.h>

#ifdef SPINXML_EVENTS
#include <wx/string.h>
#include <gui/Event.hpp>
#endif

namespace SpinXML {


void SetSchemaLocation(const char* loc);

const long END=-1;

class SpinSystem;
class Spin;
class Orientation;
class Interaction;



class ISpinSystemLoader {
public:
  enum FilterType {
    LOAD,
    SAVE,
    LOADSAVE
  };
  virtual FilterType GetFilterType() const=0;
  virtual const char* GetFilter() const=0;
  virtual const char* GetFormatName() const=0;
  virtual void LoadFile(SpinSystem* ss,const char* filename) const=0;
  virtual void SaveFile(const SpinSystem* ss,const char* filename) const=0;
};



  ///Class representing one of the shielding paramiters such as the
  ///chemical shift or J coupling
class Interaction {
  public:
  ///Constructs an undefined interaction. The type are returned by
  ///GetType() is null UNDEFINED.
    Interaction();
  ///Copy constructor
    Interaction(const Interaction& inter, SpinSystem* newSystem=NULL);
  ///Destructor
    ~Interaction();
    
  ///Print the interaction to the strandard output in a human readable
  ///form.
     void Dump() const;

  ///Enumeration of the agebrake forms
  enum Form {
    LINEAR,
    BILINEAR,
    QUADRATIC,
    ANY_FORM
  };

  ///Enumeration of the storage conventions used by this interaction
    enum Type {
        UNDEFINED,
        SCALAR,
        MATRIX,
        EIGENVALUES,
        AXRHOM,
        SPANSKEW
    };

  ///Enumeration what the physical source of this interaction is. Can
  ///be used as a hint for simulation software or it might determine
  ///how a partcular interaction is visualised.
    enum SubType {
      ST_ANY,
      ST_NMR,
      ST_EPR,

      //EPR INTERACTIONS
      ST_HFC,
      ST_G_TENSER,
      ST_ZFS,
      ST_EXCHANGE,

      //NMR INTERACTIONS
      ST_SHIELDING,
      ST_SCALAR,   

      //Interactions relevent to both nmr and epr
      ST_QUADRUPOLAR,
      ST_DIPOLAR,
      ST_CUSTOM_LINEAR,
      ST_CUSTOM_BILINEAR,
      ST_CUSTOM_QUADRATIC
    };

  ///Get a human readable name for a member of the enum Type
    static const char* GetTypeName(Type t);
  ///Get a human readable name for a member of the enum SubType
    static const char* GetSubTypeName(SubType st);

  ///Get the storage convention being used
    Type GetType() const;
  ///Get the physical source of this interaction
    SubType GetSubType() const;
  ///Set a flag indicating the physical source of this interaction.
    void SetSubType(SubType st);
  ///Returns true if the physical source of this interaction is t. The
  ///members ST_NMR, ST_EPR and ST_ANY may be used here and will be
  ///interpreted coorectly. For example, if inter is of SubType
  ///ST_SCALAR then inter.IsSubType(ST_NMR) is true.
    bool IsSubType(SubType t) const;
    
  ///If the scalar sorage convention being used then this function will set
  ///the value of its argument to the appropreate value. Otherwise the
  ///result is undefined.
    void GetScalar(double* Scalar) const;
  ///If the Matrix sorage convention being used then this function will set
  ///the value of its argument to the appropreate value. Otherwise the
  ///result is undefined.
    void GetMatrix(Matrix3* OutMatrix) const;
  ///If the Eigenvalues sorage convention being used then this function will set
  ///the value of its arguments to the appropreate value. Otherwise the
  ///result is undefined.
    void GetEigenvalues(double* XX,double* YY, double* ZZ, Orientation* OrientOut) const;
  ///If the Axiality-Rhombicity sorage convention being used then this function will set
  ///the value of its arguments to the appropreate value. Otherwise the
  ///result is undefined.
    void GetAxRhom(double* iso,double* ax, double* rh, Orientation* OrientOut) const;
  ///If the Span-Skew sorage convention being used then this function will set
  ///the value of its arguments to the appropreate value. Otherwise the
  ///result is undefined.
    void GetSpanSkew(double* iso,double* Span, double* Skew, Orientation* OrientOut) const;

  ///Set the value of the interaction using the scalar covention.
    void SetScalar(double Scalar);
  ///Set the value of the interaction using the matrix covention.
    void SetMatrix(const Matrix3& InMatrix);
  ///Set the value of the interaction using the eigenvalue covention.
    void SetEigenvalues(double XX,double YY, double ZZ, const Orientation& Orient);
  ///Set the value of the interaction using the axiality-rhombicity covention.
    void SetAxRhom(double iso,double ax, double rh, const Orientation& Orient);
  ///Set the value of the interaction using the span-skew covention.
    void SetSpanSkew(double iso,double Span, double Skew, const Orientation& Orient);

  ///Cache the form of the interaction
  bool SetLinear();   
  bool SetBilinear(); 
  bool SetQuadratic();

  bool GetIsLinear();   
  bool GetIsBilinear(); 
  bool GetIsQuadratic();

  ///Get the isotropic value of the interaction
    double GetAsScalar() const;
  ///Get the interaction as a full matrix
    Matrix3 GetAsMatrix() const /*throw(logic_error)*/;
  sigc::signal<void> sigChange;
  sigc::signal<void,Interaction*> sigDying;

  private:
    union  {
      double mScalar;
      Matrix3* mMatrix;
      struct {
        double XX;
        double YY;
        double ZZ;
      } mEigenvalues;
      struct {
	double iso;
	double ax;
	double rh;
      } mAxRhom;
      struct {
          double iso;
          double span;
          double skew;
      } mSpanSkew;
   } mData;

  Orientation mOrient;

   Type mType;
   SubType mSubType;
private:
  Spin* mSpin1;
  Spin* mSpin2;
public:
  Spin* GetSpin1() const {return mSpin1;}
  Spin* GetSpin2() const {return mSpin2;}

  void SetSpin1(Spin* s1) {sigChange();mSpin1=s1;}
  void SetSpin2(Spin* s2) {sigChange();mSpin2=s2;}


};

  ///A class representing a spin in a spin system
class Spin {
private:
  public:  
    Spin(Vector3 mPosition,std::string mLabel,long atomicNumber=1);
    Spin(const Spin& s);
    ~Spin();
  
    Vector3& GetPosition();
    void SetPosition(Vector3 Position);
    void GetCoordinates(double* _x,double* _y, double* _z) const;
    void SetCoordinates(double _x,double _y, double _z);

    void SetLabel(std::string Label);
    const char* GetLabel() const;

    std::vector<Interaction*> GetInteractions() const {return mInter;}
    void InsertInteraction(Interaction* _Interaction,long Position=END);
    void RemoveInteraction(long Position);
    void RemoveInteraction(Interaction* _Interaction);

    double GetLinearInteractionAsScalar(Interaction::SubType t=Interaction::ST_ANY) const;
    double GetQuadrapolarInteractionAsScalar(Interaction::SubType t=Interaction::ST_ANY) const;

    Matrix3 GetLinearInteractionAsMatrix(Interaction::SubType t=Interaction::ST_ANY) const;
    Matrix3 GetQuadrapolarInteractionAsMatrix(Interaction::SubType t=Interaction::ST_ANY) const;

    long GetElement() const;
    void SetElement(long element);
    std::vector<long> GetIsotopes() const;
    void SetIsotopes(std::vector<long> isotopes) const;

  sigc::signal<void> sigChange;
  sigc::signal<void,Spin*> sigDying;

  private:
    std::vector<Interaction*> mInter;
    Vector3 mPosition;
    std::string mLabel;
    long mElement;
    std::vector<long> mIsotopes;
};




class SpinSystem : public sigc::trackable {
  public:
    SpinSystem();
    SpinSystem(const SpinSystem& system);
    ~SpinSystem();

    void Clear();
    
    void Dump() const;

    long GetSpinCount() const;
    Spin* GetSpin(long n) const;
    long GetSpinNumber(Spin* spin) const;

    long GetInteractionCount() const;
    Interaction* GetInteraction(long n) const;

    std::vector<Spin*> GetSpins() const;
    void InsertSpin(Spin* _Spin,long Position=END);
    void RemoveSpin(long Position);
    void RemoveSpin(Spin* _Spin);

    void LoadFromFile(const char* filename,ISpinSystemLoader* loader);
    void SaveToFile(const char* filename,ISpinSystemLoader* saver) const;

  //Event Handlers
  void OnSpinDeleted(Spin* spin){RemoveSpin(spin);}


  std::vector<Interaction*>& GetInteractions()  {return mBilinInter;}

  sigc::signal<void,Spin*,long> sigNewSpin;
  sigc::signal<void,Spin*,Spin*> sigNewBilinear;
  sigc::signal<void> sigReloading;
  sigc::signal<void> sigReloaded;
  sigc::signal<void> sigDying;
  
  private:
    friend class Spin;

  //Set just before deleting a spin
  Spin* mIgnoreSpinKill;
  std::vector<Spin*> mSpins;
  //Set just before deleting an interaction
  Interaction* mIgnoreInterKill;
  std::vector<Interaction*> mBilinInter;
};

}; //End Namespace


#endif
