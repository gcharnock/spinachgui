
#ifndef __MATHTYPES___H_
#define __MATHTYPES___H_

#include <cstring>
#include <string>
#include <shared/unit.hpp>
#include <boost/variant.hpp>
#include <Eigen/Dense>
#include <Eigen/Geometry> 

using namespace Eigen;

#define PI 3.141592653589793238462643
using std::complex;

namespace SpinXML {

    ///Plank's constant in SI units
    extern const double hbar;

    ///Bohr magneton in SI units
    extern const double bohr_mag;
    
    ///Ther permeability of free space
    extern const double mu0;

	///Helper function to make a matrix from a bunch of doubles
	Matrix3d MakeMatrix3d(double a00, double a01, double a02,
						  double a10, double a11, double a12,
						  double a20, double a21, double a22);

    ///Euler angle type
    struct EulerAngles {
        double alpha,beta,gamma;
        EulerAngles(double _alpha,double _beta, double _gamma) 
            : alpha(_alpha),beta(_beta),gamma(_gamma) {

        }
        void Normalize() {
            
        }
        EulerAngles Normalized() const {
            
        }

    };

    ///Class for storing a 3 dimentional rotation
    class Orientation {
    public:
		///Constructs an orientation from euler angles
		Orientation(const EulerAngles& ea) : mData(ea) {}
		///Constructs an orientation from a matrix
		Orientation(const Matrix3d& m) : mData(m) {}
		///Constructs an orientation from a quaternion
		Orientation(const Quaterniond& q) : mData(q) {}
		///Constructs an orientation from an AngleAxis
		Orientation(const AngleAxisd& aa) : mData(aa) {}
		///Destructor
		~Orientation() {};
    
		///Enumeration of the four conventions on storing rotations
		enum Type {
			EULER,
			ANGLE_AXIS,
			QUATERNION,
			EIGENSYSTEM
		};
    
		///Normalise the contained rotation in place.
        void Normalize();
        ///Normalise the contained rotation.
        Orientation Normalized() const;

		///Returns the convention that is being used to store the rotation
		Type GetType() const;

		///If the euler angle convention is being used then this function
		///will set its arguments to the stored values otherwise the result
		///is undefined.
		void GetEuler(double* alpha,double* beta,double* gamma) const;
		///If the angle axis convention being used then this function will
		///set its arugments to those values otherwise the result is
		///undefined.
		void GetAngleAxis(double* angle,Vector3d* axis) const;
		///If the quaternion convention being used then this function will
		///set its arguments to those values otherwise the result is
		///undefined.
		void GetQuaternion(double* real, double* i, double* j, double* k) const;
		///If the eigensystem convention is being used then this function
		///will set its arguments to the appropriate values otherwise the
		///result is undefined.
		void GetEigenSystem(Vector3d* XAxis,Vector3d* YAxis, Vector3d* ZAxis) const;

		const Orientation& operator=(const EulerAngles& ea);
		const Orientation& operator=(const AngleAxisd& aa);
		const Orientation& operator=(const Matrix3d& m);
		const Orientation& operator=(const Quaterniond& q);

		///Converts to a rotation matrix
		Matrix3d GetAsMatrix() const;

		///returns a representation of the orientation as a string that is
		///both machiene parsable and human readable.
		std::string ToString() const;
		///Parses an orentation from a string representation. The form of
		///the representation should be the same as returned by ToString
		void FromString(std::string string);

        Orientation GetAsEuler() const;
        Orientation GetAsEigenSystem() const;
        Orientation GetAsAngleAxis() const;
        Orientation GetAsQuaternion() const;

    private:
        typedef boost::variant<EulerAngles,AngleAxisd,Quaterniond,Matrix3d> var_t;
        var_t mData;
    };
};
#endif
