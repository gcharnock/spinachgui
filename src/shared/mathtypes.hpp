
#ifndef __MATHTYPES___H_
#define __MATHTYPES___H_

#include <string>
#include <cmath>

namespace SpinXML {

    ///Plank's constant in SI units
    extern const double hbar;

    ///Bohr magneton in SI units
    extern const double bohr_mag;
    
    ///Ther permeability of free space
    extern const double mu0;

    ///Class for storing a 3 dimensional vector
    template<typename T>
    class Vector3_t {
    public:
        ///Constructs an uninitalised vector
        Vector3_t(){}
        ///Copy Constructor
        Vector3_t(const Vector3_t& v) 
            :  x(v.x),y(v.y),z(v.z) {
        }
        ///Constructs a new vector
        Vector3_t(T _x,T _y,T _z) 
            : x(_x),y(_y),z(_z) {
        }

        ///Sets it's arguments to the value of the vector's z,y and z
        ///components
        void GetCoordinates(T* _x,T* _y, T* _z) const {
            *_x=x;
            *_y=y;
            *_z=z;
        }
        ///Sets it's arguments to the value of the vector's z,y and z
        ///components in SI units
        void GetCoordinatesSI(double* _x,double* _y, double* _z) const {
            *_x=x.si;
            *_y=y.si;
            *_z=z.si;
        }
        ///Sets the vector's x,y and z coordinates
        void SetCoordinates(T _x,T _y, T _z) {
            x=_x;
            y=_y;
            z=_z;
        }
        ///Sets the vector's x,y and z coordinates in SI units
        void SetCoordinatesSI(double _x,double _y, double _z) {
            x.si=_x;
            y.si=_y;
            z.si=_z;
        }

        ///Get the X component 
        T GetX() const {return x;}
        ///Get the Y component 
        T GetY() const {return y;}
        ///Get the Z component 
        T GetZ() const {return z;}

        ///Set the X component 
        void SetX(T _x) {x=_x;}
        ///Set the Y component 
        void SetY(T _y) {y=_y;}
        ///Set the Z component 
        void SetZ(T _z) {z=_z;}

        ///Returns the magnitude of the vector
        T length() const {
            return sqrt(x*x+y*y+z*z);
        }

        ///normalises the vector
        void normalise() {
            double inv_length=1/sqrt(x.si*x.si+y.si*y.si+z.si*z.si);
            x*=inv_length;
            y*=inv_length;
            z*=inv_length;                
        }

        Vector3_t operator+(const Vector3_t& v) const {
            return Vector3_t(x+v.x,y+v.y,z+v.z);
        }
        Vector3_t operator-(const Vector3_t& v) const {
            return Vector3_t(x-v.x,y-v.y,z-v.z);
        }

    private:
        T x;
        T y;
        T z;
    };
    typedef Vector3_t<double> Vector3;

    ///Class for storeing a 3x3 matrix.
    class Matrix3 {
    public:
        ///Default constructor. Constructs the identity
        Matrix3();
        ///Constructs a specific 3x3 matrix
        Matrix3(double a00,double a01,double a02,
                double a10,double a11,double a12,
                double a20,double a21,double a22);
        ///Copies the matrix
        Matrix3(const Matrix3& m);

        ///Print the matrix to the standard output in a human readable
        ///format
        void Dump() const;

        ///Get the trace of the matrix
        double Trace() const {return raw[0]+raw[4]+raw[8];}

        ///Get the transpose of the matrix
        Matrix3 Transpose() const;
    
        ///Get a pointer to the matrix in memory. The matrix is stored in
        ///row major form (GetRaw()[0]=a00,GetRaw()[1]=a01, etc.)
        const double* GetRaw() const;
    
        ///Get a specific value by row and column index.
        double Get(long column, long row) const;
        ///Get a specific value by row and column
        double operator() (long column, long row) const;
        ///Add together two matricese in the standard fasion.
        Matrix3 operator+ (const Matrix3& m) const;
        ///Multiply two matricese together
        Matrix3 operator* (const Matrix3& m) const;
        ///Multiply a matrix my a scalar
        Matrix3 operator* (double s) const;
        ///Multiply a matrix my a scalar
        Matrix3 operator*= (double s);
        ///Add together two matricese in the standard fasion and set the
        ///value of the first to the result
        Matrix3& operator+= (const Matrix3& m);
        ///Set the value of one matrix to the value of the second.
        Matrix3& operator= (const Matrix3& m);
        ///Set a specific element to the value of val
        void Set(long column,long row,double val);
    private:
        double raw[9];
    };

    ///Class for storeing a 4x4 matrix. Mostly used for graphics, but
    ///could be useful elsewhere...
    class Matrix4 {
    public:
        ///Default constructor. Constructs the identity
        Matrix4();
        ///Constructs a specific 3x3 matrix
        Matrix4(double a00,double a01,double a02,double a03,
                double a10,double a11,double a12,double a13,
                double a20,double a21,double a22,double a23,
                double a30,double a31,double a32,double a33);
        ///Copies the matrix
        Matrix4(const Matrix4& m);

        ///Print the matrix to the standard output in a human readable
        ///format
        void Dump() const;

        ///Get the trace of the matrix
        double Trace() const {return raw[0]+raw[5]+raw[10]+raw[15];}

        ///Get the transpose of the matrix
        Matrix4 Transpose() const;
    
        ///Get a pointer to the matrix in memory. The matrix is stored in
        ///row major form (GetRaw()[0]=a00,GetRaw()[1]=a01, etc.)
        const double* GetRaw() const;
    
        ///Get a specific value by row and column index.
        double Get(long column, long row) const;
        ///Get a specific value by row and column
        double operator() (long column, long row) const;
        ///Add together two matricese in the standard fasion.
        Matrix4 operator+ (const Matrix4& m) const;
        ///Multiply two matricese together
        Matrix4 operator* (const Matrix4& m) const;
        ///Add together two matricese in the standard fasion and set the
        ///value of the first to the result
        Matrix4& operator+= (const Matrix4& m);
        ///Set the value of one matrix to the value of the second.
        Matrix4& operator= (const Matrix4& m);
        ///Set a specific element to the value of val
        void Set(long column,long row,double val);
    private:
        double raw[16];
    };

    ///Class for storing a 3 dimentional rotation
    class Orientation {
    public:
        ///Constructs and orientaion of type Orientation::UNDEFINED with all
        ///other data uninialised.
        Orientation();
        ///Copies an orientation
        Orientation(const Orientation& orient);
        ///Assignment Operation
        const Orientation& operator=(const Orientation& orient);
        ///Destructor
        ~Orientation();
    
        ///Enumeration of the four conventions on storing rotations
        enum Type {
            UNDEFINED,
            EULER,
            ANGLE_AXIS,
            QUATERNION,
            EIGENSYSTEM
        };
    
        ///Returns the convention that is being used to store the rotation
        Type GetType() const;

        ///If the euler angle convention is being used then this function
        ///will set its arguments to the stored values otherwise the result
        ///is undefined.
        void GetEuler(double* alpha,double* beta,double* gamma) const;
        ///If the angle axis convention being used then this function will
        ///set its arugments to those values otherwise the result is
        ///undefined.
        void GetAngleAxis(double* angle,Vector3* axis) const;
        ///If the quaternion convention being used then this function will
        ///set its arguments to those values otherwise the result is
        ///undefined.
        void GetQuaternion(double* real, double* i, double* j, double* k) const;
        ///If the eigensystem convention is being used then this function
        ///will set its arguments to the appropriate values otherwise the
        ///result is undefined.
        void GetEigenSystem(Vector3* XAxis,Vector3* YAxis, Vector3* ZAxis) const;

        ///Set the value of the rotation in terms of euler angles. This
        ///function also sets the type value as returned by GetType()
        void SetEuler(double alpha,double beta,double gamma);
        ///Set the value of the rotation in terms of the angle axis
        ///paramiters. This function also sets the type value as returned by
        ///GetType()
        void SetAngleAxis(double angle,const Vector3& axis);
        ///Set the value of the rotation in terms of quaternion
        ///paramiters. This function also sets the type value as returned by
        ///GetType()
        void SetQuaternion(double real, double i, double j, double k);
        ///Set the value of the rotation in terms of eigensystem. This
        ///function also sets the type value as returned by GetType()
        void SetEigenSystem(const Vector3& XAxis,const Vector3& YAxis, const Vector3& ZAxis);

        ///Converts to a rotation matrix
        Matrix3 GetAsMatrix() const;

        ///returns a representation of the orientation as a string that is
        ///both machiene parsable and human readable.
        std::string ToString() const;
        ///Parses an orentation from a string representation. The form of
        ///the representation should be the same as returned by ToString
        void FromString(std::string string);
    private:
        void Clear();
        union  {
            struct {
                double alpha;
                double beta;
                double gamma;
            } mEuler;
            struct {
                double angle;
                Vector3* axis;
            } mAngleAxis;
            struct {
                double real;
                double i;
                double j;
                double k;
            } mQuaternion;
            struct {
                Vector3* XAxis;
                Vector3* YAxis;
                Vector3* ZAxis;
            } mEigenSystem;
        } mData;
        Type mType;
    };


};
#endif
