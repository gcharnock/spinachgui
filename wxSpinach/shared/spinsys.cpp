
#include "spinsys.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

SpinsysXMLRoot::SpinsysXMLRoot() : mXMLSpinSys(new SpinachSpinsys()) {
  cout << "The size of a newly created set of spins is " << mXMLSpinSys->getSpin().size() << endl;
}

void SpinsysXMLRoot::loadFromFile(const char* filename) {
  clear();
  try {
    auto_ptr<Spin_system> tmpPtr(parseSpin_system(filename));
    mXMLSpinSys.reset(new SpinachSpinsys(*tmpPtr));
  } catch(const xml_schema::Exception& e) {
    cerr << e << endl;
    exit(1);
  }
}

void SpinsysXMLRoot::saveToFile(const char* filename) const {
  xml_schema::NamespaceInfomap map;
  map[""].name = "";
  map[""].schema = "../data/spinsys_spec.xsd";

  ofstream fout(filename);
  if(!fout.is_open()) {
    cerr << "Could not open " << filename << endl;
    return;
  }
  Spin_system ss(*mXMLSpinSys);
  cout << "About to save" << endl;
  try {
    serializeSpin_system(fout, ss, map);
  } catch (const xml_schema::Exception& e) {
    cerr << e << endl;
    exit(1);
  }
}

void SpinsysXMLRoot::loadFromG03File(const char* filename) {
  clear();
  mXMLSpinSys->loadFromG03File(filename);
}

void SpinsysXMLRoot::loadFromXYZFile(const char* filename) {
  clear();
  mXMLSpinSys->loadFromXYZFile(filename);
}

void SpinsysXMLRoot::clear() {
  mXMLSpinSys.reset(new SpinachSpinsys());
}


//============================================================//
//

void SpinachSpinsys::dump() const {
  cout << "Printing out a spin system" << endl;
  cout << "Spins:" << endl;
  for (SpinConstIterator i(getSpin().begin()); i != getSpin().end(); ++i) {
    cout << " Spin name=" << i->getLabel() << endl;
    cout << "      number=" << i->getNumber() << endl;
    cout << "      isotope=" << i->getIsotope() << endl;
    cout << "      coords=(" << i->getCoordinates().getX() << ",";
    cout << "              " << i->getCoordinates().getY() << ",";
    cout << "              " << i->getCoordinates().getZ() << ")" << endl;
  }
  for (InteractionConstIterator i(getInteraction().begin()); i != getInteraction().end();  ++i) {
    SpinachInteraction so(*i);
    cout << " Interaction type=" << so.getFormAsString() << endl;
    cout << "      Spin_1=" << so.getSpin_1() << endl;
  }
}

void SpinachSpinsys::loadFromXYZFile(const char* filename) {
  ifstream fin(filename);
  cout << "Opening an xyz file " << filename << endl;
  if(!fin.is_open()) {
    //Throw some sort of error here
    cerr << "Couhn't open the file" << endl;
  }
  SpinSequence Spins;
  string element;
  double x,y,z;
  long nAtoms=0;
  while(!fin.eof()) {
    fin >> element >> x >> y >> z >> ws;
    cout << element << " " << x << "  " << y << " " << z << endl;
    Spin s(Vector(x,y,z),nAtoms,element,0);  //Last paramiter is the reference frame
    s.setLabel("Spin");
    Spins.push_back(s);
    nAtoms++;
  } 
  setSpin(Spins);
}

void SpinachSpinsys::loadFromG03File(const char* filename) {
  /*
    This function really needs some work done on in, as it's not using C++
    streams properly. This is due to it being adapted from matlab code (which uses
    c style I/O
   */
  ifstream fin(filename);
  cout << "Opening a g03 file:" << filename << endl;
  if(!fin.is_open()) {
    //Throw some sort of error here
    cerr << "Couldn't open file" << endl;
    return;
  }
  SpinSequence Spins;
  cout << "Spins.size() = " <<  Spins.size() << endl;
  InteractionSequence Interactions;
  long nAtoms=0;
  while(!fin.eof()) {
    string line;
    char buff[500];  //TODO buletproof this
    fin.getline(buff,500); line=buff; //Read a line
    boost::algorithm::trim(line); //Remove whitespace

    if (line=="Standard orientation:") {
      cout << "Standard orientation found." << endl;
      //Skip 4 lines
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      istringstream stream;

      fin.getline(buff,500); line=buff; stream.str(line); //Read a line
      while(line.find("--------") == string::npos && !fin.eof()) {
	int dummy1,dummy2,dummy3;
	double x,y,z;
	stream >> dummy1 >> dummy2 >> dummy3 >> x >> y >> z;
	stream.clear();
	fin.getline(buff,500); line=buff; stream.str(line); //Read a line
	Spin s(Vector(x,y,z),nAtoms,"H1",0); //Assume everything is a hydrogen, overwrite later
	
	s.setLabel("A Spin");
	Spins.push_back(s);
	nAtoms++;
      }
    }
    if (line=="Isotropic Fermi Contact Couplings") {
      cout << "Isotropic couplings found" << endl;
      //Skip a line
      fin.getline(buff,500); line=buff; //Read a line
      istringstream stream;
      for (long i=0;i<nAtoms;i++) {
	string dummy1,dummy2,dummy3;
	double isoCoupling;
	fin.getline(buff,500); line=buff; //Read a line
	stream.clear(); stream.str(line);
	//Read the coupling strength (in megaherz)
	stream >> dummy1 >> dummy2 >> dummy3 >> isoCoupling;
	Interaction1 inter("Isotropic","MHz",i,0); //Last paramiter is reference frame, which is always lab
	inter.setScalar(isoCoupling);
	Interactions.push_back(inter);
      }          
    }
    if(line=="Anisotropic Spin Dipole Couplings in Principal Axis System") {
      cout << "Anisotropic couplings found" << endl;
      //Skip 4 lines
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      fin.getline(buff,500); line=buff; //Read a line
      istringstream stream;
      for (long i=0;i<nAtoms;i++) {
	string dummy1,dummy2,dummy3,dummy4,dummy5,isotope;
	double eigenvalue1,eigenvalue2,eigenvalue3;
	double x1,y1,z1,x2,y2,z2,x3,y3,z3;
	//Read the rotation matrix and eigenvalues (in megaherz)
	fin.getline(buff,500); line=buff; stream.clear(); stream.str(line); //Read a line
	stream                      >> dummy1 >> dummy2 >> eigenvalue1 >> dummy3 >> dummy4 >> x1 >> y1 >> z1;

	fin.getline(buff,500); line=buff; stream.clear(); stream.str(line); //Read a line
	stream >> dummy5 >> isotope >> dummy1 >> dummy2 >> eigenvalue2 >> dummy3 >> dummy4 >> x2 >> y2 >> z2;

	fin.getline(buff,500); line=buff; stream.clear(); stream.str(line); //Read a line
	stream                      >> dummy1 >> dummy2 >> eigenvalue3 >> dummy3 >> dummy4 >> x3 >> y3 >> z3;
	//Skip a line
	fin.getline(buff,500); line=buff; //Read a line

	Interaction1 inter("Isotropic","MHz",i,0); //Last paramiter is reference frame, which is always lab
	inter.setEigenvalues(Eigenvalues(eigenvalue1*0.05,eigenvalue2*0.05,eigenvalue3*0.05));
	Orientation o;
	o.setEigensystem(Eigensystem(Vector(x1,y1,z1),Vector(x2,y2,z2),Vector(x3,y3,z3)));
	inter.setOrientation(o);
	Interactions.push_back(inter);

	//Now process the isotope
	long leftBracket=isotope.find("(");
	long rightBracket=isotope.find(")");
	string element=isotope.substr(0,leftBracket);
	string massnum=isotope.substr(leftBracket+1,rightBracket-leftBracket-1);
	isotope=element+massnum;
	Spins[i].setIsotope(isotope);
      }
    }
  }
  cout << "Finished loading the g03 file, saving spins.size()=" << Spins.size() << endl;
  setSpin(Spins);
  cout << " and getSpin().size()=" << getSpin().size() << endl;
  setInteraction(Interactions);
}

void SpinachSpinsys::addSpin() {
  //  Spin s();
  //  mXMLSpinSys->getSpin().push_back(s);
}

long SpinachSpinsys::getSpinCount() const {
  return getSpin().size();
}



SpinachSpin SpinachSpinsys::getSpinByIndex(long n) {
  if(n>=0 && n<getSpin().size()) {
    return getSpin()[n];
  }
  SpinachSpin s;
  return s;
}

SpinachInteraction SpinachSpinsys::getInteractionByIndex(long n) {
  if(n>=0 && n<getInteraction().size()) {
    return getInteraction()[n];
  }
  SpinachInteraction inter;
  return inter;
}

long SpinachSpinsys::getInteractionCount() const {
  return getInteraction().size();
}

std::vector<long> SpinachSpinsys::getNearbySpins(long spinNumber,double distance) {
  SpinSequence Spins(getSpin());
  std::vector<long> result;
  double dist2=distance*distance;
  Vector coords1=Spins[spinNumber].getCoordinates();
  double x1=coords1.getX();
  double y1=coords1.getY();
  double z1=coords1.getZ();

  for(long i=spinNumber+1;i<getSpin().size();i++) {
    Vector coords2=Spins[i].getCoordinates();
    double x2=coords2.getX();
    double y2=coords2.getY();
    double z2=coords2.getZ();
    double deltaX=(x1-x2);
    double deltaY=(y1-y2);
    double deltaZ=(z1-z2);
    if(deltaX*deltaX+deltaY*deltaY+deltaZ*deltaZ < dist2) {
      result.push_back(i);
    }
  }
  return result;
}

Matrix3 SpinachSpinsys::GetTotalInteractionOnSpinAsMatrix(long n) {
  Matrix3 totalMatrix(0,0,0,0,0,0,0,0,0);
  for(long i=0;i < getInteraction().size();++i) {
    if(getInteraction()[i].getSpin_1()==n) {
      totalMatrix=totalMatrix+SpinachInteraction(getInteraction()[i]).getAsMatrix();
    }
  }
  return totalMatrix;
}



//============================================================//
// SpinachSpin

Vector SpinachSpin::getCoords() {
  return getCoordinates();
}

void SpinachSpin::dump() {
  cout << "Spin name=" << getLabel() << endl;
  //const Spin::CoordinatesType c = getCoordinates();
  //cout << c.getX() << endl;
  //cout << c.getY() << endl;
  //cout << c.getZ() << endl;
  //cout << "IsotopeE = " << getIsotope() << endl;
}


//============================================================//
// SpinachOrientation

long SpinachOrientation::getForm() const {
  if(getEuler_angles().present()) {
    return ORIENT_EULER;
  } else if(getAngle_axis().present()) {
    return ORIENT_ANGLE_AXIS;
  } else if(getQuaternion().present()) {
    return ORIENT_QUATERNION;
  } else if(getEigensystem().present()) {
    return ORIENT_EIGENSYSTEM;
  } else {
    //TODO Throw some sort of error here
  }
}


Matrix3 SpinachOrientation::getAsMatrix() const {
  if(getForm()==ORIENT_EIGENSYSTEM) {
    Eigensystem sys=getEigensystem().get();
    Vector xa=sys.getX_axis();
    Vector ya=sys.getY_axis();
    Vector za=sys.getZ_axis();

    return Matrix3(xa.getX(),ya.getX(),za.getX(),
		   xa.getY(),ya.getY(),za.getY(),
		   xa.getZ(),ya.getZ(),za.getZ());
  }
  return Matrix3(1,0,0,0,1,0,0,0,1);
}


//============================================================//
// SpinachInteraction



SpinachOrientation SpinachInteraction::getSpinachOrientation() {
  OrientationOptional orentOpt=Interaction1::getOrientation();
  if(orentOpt.present()) {
    SpinachOrientation orient(orentOpt.get());
    return orient;
  } else {
    //TODO Return some sort of exception to the calling code at this point
    cerr << "Trying to get the orientation when this type of interaction doesn't need it" << endl;
    const SpinachOrientation so;
    return so;
  }
};

Matrix3 SpinachInteraction::getAsMatrix() const {
  if(getForm()==INTER_SCALER) {
    //Return the identity multipled by the scalar
    double s=getScalar().get();
    Matrix3 m(s,0,0,0,s,0,0,0,s);
    return m;
  } else if(getForm()==INTER_MATRIX) {
    //Just return the matrix
    Matrix3 m(getMatrix().get());
    return m;
  } else if(getForm()==INTER_EIGENVALUES) {
    //Convert to a matrix

    const Eigenvalues ev=getEigenvalues().get();

    double xx=ev.getXX();
    double yy=ev.getYY();
    double zz=ev.getZZ();

    const SpinachOrientation o=getOrientation().get();
    Matrix3 intMatrix=o.getAsMatrix();

    intMatrix.set(0,0,intMatrix.get(0,0)*xx);
    intMatrix.set(0,1,intMatrix.get(0,1)*yy);
    intMatrix.set(0,2,intMatrix.get(0,2)*zz);

    intMatrix.set(1,0,intMatrix.get(1,0)*xx);
    intMatrix.set(1,1,intMatrix.get(1,1)*yy);
    intMatrix.set(1,2,intMatrix.get(1,2)*zz);

    intMatrix.set(2,0,intMatrix.get(2,0)*xx);
    intMatrix.set(2,1,intMatrix.get(2,1)*yy);
    intMatrix.set(2,2,intMatrix.get(2,2)*zz);
    return intMatrix;
  } else {
    cerr << "Interaction type not suported in getAsMatrix()" << endl;
  }
  //Return the identity
  Matrix3 identity(0,0,0,0,0,0,0,0,0);
  return identity;
}

long SpinachInteraction::getForm() const {
  if(getScalar().present()) {
    return INTER_SCALER;
  } else if(getMatrix().present()) {
    return INTER_MATRIX;
  } else if(getEigenvalues().present()) {
    return INTER_EIGENVALUES;
  } else if(getAxiality_rhombicity().present()) {
    return INTER_AXIALITY_RHOMBICITY;
  } else if(getSpan_skew().present()) {
    return INTER_SPAN_SKEW;
  } else {
    //TODO Throw some sort of error here
  }
}

const char* SpinachInteraction::getFormAsString() const {
  if(getScalar().present()) {
    return "scalar";
  } else if(getMatrix().present()) {
    return "matrix";
  } else if(getEigenvalues().present()) {
    return "eigenvalues";
  } else if(getAxiality_rhombicity().present()) {
    return "axiality_rhombicity";
  } else if(getSpan_skew().present()) {
    return "span_skew";
  } else {
    //TODO Throw some sort of error here
  }
}

//============================================================//
// SpinachFrame

SpinachFrame::SpinachFrame(const Reference_frame& _rf) : Reference_frame(_rf) {
  
}


  //============================================================//

Matrix3::Matrix3(double a00,double a01,double a02,
		 double a10,double a11,double a12,
		 double a20,double a21,double a22) {
  ElementSequence elements;
  elements.resize(9);
  elements[0]=a00;
  elements[1]=a01;
  elements[2]=a02;

  elements[3]=a10;
  elements[4]=a11;
  elements[5]=a12;

  elements[6]=a20;
  elements[7]=a21;
  elements[8]=a22;
  setElement(elements);
}

void Matrix3::dump() const {
  const ElementSequence elements=getElement();
  double a00=elements[0];
  double a01=elements[1];
  double a02=elements[2];

  double a10=elements[3];
  double a11=elements[4];
  double a12=elements[5];

  double a20=elements[6];
  double a21=elements[7];
  double a22=elements[8];

  cout << "Matrix3:" << endl;
  cout << " (" << a00 << " " << a01 << " " << a02 << endl;
  cout << " (" << a10 << " " << a11 << " " << a12 << endl;
  cout << " (" << a20 << " " << a21 << " " << a22 << ")" << endl;
}

Matrix3 Matrix3::operator+(const Matrix3& m) const {
  //Well unlike with a serious matrix class, this doesn't have to be efficent
  Matrix3 retVal(get(0,0)+m.get(0,0),
		 get(0,1)+m.get(0,1),
		 get(0,2)+m.get(0,2),
		 get(1,0)+m.get(1,0),
		 get(1,1)+m.get(1,1),
		 get(1,2)+m.get(1,2),
		 get(2,0)+m.get(2,0),
		 get(2,1)+m.get(2,1),
		 get(2,2)+m.get(2,2));
  return retVal;
}
