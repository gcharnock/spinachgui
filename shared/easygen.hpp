
#ifndef __EASYSPIN_CODEGN_H__
#define __EASYSPIN_CODEGN_H__

#include <string>
#include <map>
#include <boost/optional.hpp>

namespace SpinXML {
    class SpinSystem;
};

struct EasySpinInput {
    EasySpinInput() {
    }

    std::string generate(SpinXML::SpinSystem* spinsys) const;

    //================================================================================//
    // EasySpin function
    enum EasySpinFunc {
        GARLIC,
        CHILI,
        PEPPER
    };
    void setEasySpinFunction(EasySpinFunc easySpinFunc) {
        mEasySpinFunc = easySpinFunc;
    }
    EasySpinFunc mEasySpinFunc;

    //================================================================================//
    // Experiment
    void setCentreSweep(double centre,double sweep) {
        mCentre = centre;
        mSweep = sweep;
    }
    double mCentre;
    double mSweep;

    //----------------------------------------//
    void setMWFreq(double mwFreq) {
        mMWFreq = mwFreq;
    }
    double mMWFreq;

    //----------------------------------------//
    void setTemperature(double temperature) {
        mTemperature = temperature;
    }
    boost::optional<double> mTemperature;
    
    //----------------------------------------//
    void setModAmp(double modAmp) {
        mModAmp = modAmp;
    }
    boost::optional<double> mModAmp;


    //----------------------------------------//
    void setNPoints(unsigned long nPoints) {
        mNPoints = nPoints;
    }
    unsigned long mNPoints;
    //----------------------------------------//
    void setHarmonic(unsigned long harmonic) {
        mHarmonic = harmonic;
    }
    unsigned long mHarmonic;
    //----------------------------------------//
    enum Mode {
        PARALLEL,
        PERPENDICULAR
    };
    void setMode(Mode mode) {
        mMode = mode;
    }
    Mode mMode;
    //----------------------------------------//

    //Should only be set iff mSampleState = Cystal
    void setSpaceGroup(unsigned long spaceGroup) {
        mSpaceGroup = spaceGroup;
    }
    boost::optional<unsigned long> mSpaceGroup;
    //----------------------------------------//

    //TODO: Crystal Orientations

    //----------------------------------------//
    //Valid range is 0 to 2Pi
    void setMWPhase(unsigned long mwPhase) {
        mMWPhase = mwPhase;
    }
    double mMWPhase;

    //================================================================================//
    // Options
    enum Method {
        MATRIX,
        PERT1,
        PERT2
    };
    void setMethod(Method method) {
        mMethod = method;
    }
    Method mMethod;

    void setNKnots(unsigned long nKnots) {
        mNKnots = nKnots;
    }
    //0 => unused/default
    unsigned long mNKnots;

    void setInterpolate(unsigned long interpolate) {
        mInterpolate = interpolate;
    }
    //0 => no interpolation
    unsigned long mInterpolate;

    enum Output {
        SUMMED,
        SEPERATE
    };
    void setOutput(Output output) {
        mOutput = output;
    }
    Output mOutput;

    //============================================================//
    // Broadernings
    double mGaussianFWHM;
    double mLorentFWHM;

    double mHStrainX,mHStrainY,mHStrainZ;        
    double mGStrainX,mGStrainY,mGStrainZ;        
    double mAStrainX,mAStrainY,mAStrainZ;        
    double mDStrainD,mDStrainE,mDStrainCorrCoeff;

    //Static stuff
    static void staticCtor() {
        mModeNames[PARALLEL] = "parallel";
        mModeNames[PERPENDICULAR] = "perpendicular";

        mMethodNames[MATRIX] = "matrix?";
        mMethodNames[PERT1] = "pert1?";
        mMethodNames[PERT2] = "pert2?";

        mOutputNames[SUMMED] = "summed?";
        mOutputNames[SEPERATE] = "seperate?";

        mEasySpinNames[GARLIC] = "garlic";
        mEasySpinNames[CHILI] = "chili";
        mEasySpinNames[PEPPER] = "pepper";
    }
    struct __Init {
        __Init() {
            staticCtor();
        }
    };
    static std::map<Mode  ,std::string> mModeNames;
    static std::map<Method,std::string> mMethodNames;
    static std::map<Output,std::string> mOutputNames;
    static std::map<EasySpinFunc,std::string> mEasySpinNames;


private:
    static __Init __init;
};


#endif
