%module spinsys
%{
/* Put header files here or function declarations like below */
#include <shared/spinsys.hpp>
#include <shared/spinsys_spec.hpp>
%}

%typemap(out) Spin_system::SpinSequence&
{
  int j=0;
  $result = PyList_New($1->size());
  for (Spin_system::SpinConstIterator i=$1->begin();i != $1->end(); ++i) {
    PyObject* label;
    if(i->getLabel().present()) {
      label = PyString_FromString(i->getLabel().get().c_str());
    } else {
      label = PyString_FromString("");
    }
    PyObject* isotope=PyString_FromString(i->getIsotope().c_str());
    PyObject* number =PyLong_FromLong(i->getNumber());
    const Spin::CoordinatesType c = i->getCoordinates();
    PyObject* x=PyFloat_FromDouble(c.getX());
    PyObject* y=PyFloat_FromDouble(c.getY());
    PyObject* z=PyFloat_FromDouble(c.getZ());
    PyObject* coords = PyTuple_Pack(3,x,y,x);
    PyObject* spin=PyTuple_Pack(4,number,isotope,label,coords);
    PyList_SetItem($result,j,spin);
    j++;
  }
}

%typemap(out) Vector
{
  // PyObject* x=PyFloat_FromDouble(42.0);//$1.getX());
  //PyObject* y=PyFloat_FromDouble(42.0);//$1.getY());
  //PyObject* z=PyFloat_FromDouble(42.0);//$1.getZ());
  //PyObject* t=PyTuple_Pack(3,x,y,z);
  $result = PyInt_FromLong(4);
}

//=========>> Spinsys class <<=========//

class Spinsys {
public:
  Spinsys();
  ///Create an empty spin system
  void createNew();
  ///Load a spin system from an XML file located at filename
  void loadFromFile(const char* filename);
  ///Save the spin system to an XML file at filename
  void saveToFile(const char* filename);
  ///Output the spin system in a human readable format to the standard
  ///output for debugging purposes.
  void dump() const;
  ///Get a reference to a spin
  void addSpin();
  ///Get a list of references to spins
  Spin_system::SpinSequence& getSpins();
  SpinachSpin getSpin(long n);
  long getSpinCount();
};

//=========>> Spin Class =========//

//Swig just needs to know that there is a spin class and it has
//a default constructior
class Spin {
public:
  Spin();
};


class SpinachSpin : public Spin {
public:
  SpinachSpin();
  void sayHello();
  void dump();
  Vector getCoords();
};


//=========>> Orientation Class <<=====//

class Orientation {
public:
  Orientation();
};

class SpinachOrientation : public Orientation{
public:
  SpinachOrientation() : Orientation() {}
  SpinachOrientation(const Orientation& _O) : Orientation(_O) {}
};


//=========>> Reference Frame Class <<=========//

class Reference_frame {
public:
  Reference_frame();
};

class SpinachFrame : public Reference_frame {
public:
  SpinachFrame() : Reference_frame() {}
  SpinachFrame(const Reference_frame& _rf) : Reference_frame(_rf) {}
protected:
};
