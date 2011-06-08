
#include <gui/EasySpin.hpp>
#include <wx/valtext.h>

#include <gui/TextCtrlFocus.hpp>
#include <gui/NumericValidators.hpp>
#include <fstream>
#include <vector>
#include <map>
#include <wx/log.h>
#include <shared/foreach.hpp>
#include <climits>
#include <gui/Spacegroup.hpp>
#include <boost/optional.hpp>

#include <sigc++/sigc++.h>


using namespace std;
using namespace sigc;
using namespace SpinXML;
using boost::optional;

//================================================================================//
// EasySpin struct, a structure for storing an easyspin experiment

struct EasySpinInput {
    EasySpinInput() {
    }

    string generate() const {
        ostringstream oss;

        oss << "Sys = struct('g',[2.0088 2.0061 2.0027],'Nucs','14N','A',A);" << endl;

        oss << endl;

        oss << "Exp.mwFreq = " << mMWFreq << ";" << endl;
        oss << "Exp.CenterSweep = [" << mCentre << " " << mSweep << "];" << endl;
        oss << endl;
        oss << "Exp.Harmonic = " << mHarmonic << ";" << endl;
        oss << "Exp.nPoints = "  << mNPoints << ";" << endl;
        oss << "Exp.Mode = \""     << mModeNames[mMode] << "\";" << endl;
        oss << "Exp.mwPhase = "  << mMWPhase << ";" << endl;

        oss << endl;
        if(mModAmp) {
            oss << "Exp.ModAmp = " << mModAmp.get() << ";" << endl;}
        if(mTemperature) {
            oss << "Exp.Temperature = " << mTemperature.get() << ";" << endl;}

        

        return oss.str();
    }

    //================================================================================//
    // Experiment

public:
    void setCentreSweep(double centre,double sweep) {
        mCentre = centre;
        mSweep = sweep;
    }
private:
    double mCentre;
    double mSweep;

    //----------------------------------------//

public:
    void setMWFreq(double mwFreq) {
        mMWFreq = mwFreq;
    }
private:
    double mMWFreq;

    //----------------------------------------//

public:
    void setTemperature(double temperature) {
        mTemperature = temperature;
    }
private:
    optional<double> mTemperature;
    
    //----------------------------------------//

public:
    void setModAmp(double modAmp) {
        mModAmp = modAmp;
    }
private:
    optional<double> mModAmp;


    //----------------------------------------//
public:
    void setNPoints(unsigned long nPoints) {
        mNPoints = nPoints;
    }
private:
    unsigned long mNPoints;
    //----------------------------------------//
public:
    void setHarmonic(unsigned long harmonic) {
        mHarmonic = harmonic;
    }
private:
    unsigned long mHarmonic;
    //----------------------------------------//
public:
    enum Mode {
        PARALLEL,
        PERPENDICULAR
    };
    void setMode(Mode mode) {
        mMode = mode;
    }
private:
    Mode mMode;
    //----------------------------------------//

    //Should only be set iff mSampleState = Cystal
public:
    void setSpaceGroup(unsigned long spaceGroup) {
        mSpaceGroup = spaceGroup;
    }
private:
    boost::optional<unsigned long> mSpaceGroup;
    //----------------------------------------//
public:
    //TODO: Crystal Orientations
private:
    //----------------------------------------//
    //Valid range is 0 to 2Pi
public:
    void setMWPhase(unsigned long mwPhase) {
        mMWPhase = mwPhase;
    }
private:
    double mMWPhase;

    //================================================================================//
    // Options
public:
    enum Method {
        MATRIX,
        PERT1,
        PERT2
    };
    void setMethod(Method method) {
        mMethod = method;
    }
private:
    Method mMethod;

public:
    void setNKnots(unsigned long nKnots) {
        mNKnots = nKnots;
    }
private:
    //0 => unused/default
    unsigned long mNKnots;

public:
    void setInterpolate(unsigned long interpolate) {
        mInterpolate = interpolate;
    }
private:
    //0 => no interpolation
    unsigned long mInterpolate;

public:
    enum Output {
        SUMMED,
        SEPERATE
    };
    void setOutput(Output output) {
        mOutput = output;
    }
private:
    Output mOutput;

public:
    //Static stuff
    static void staticCtor() {
        mModeNames[PARALLEL] = "parallel";
        mModeNames[PERPENDICULAR] = "perpendicular";

        mMethodNames[MATRIX] = "matrix?";
        mMethodNames[PERT1] = "pert1?";
        mMethodNames[PERT2] = "pert2?";

        mOutputNames[SUMMED] = "summed?";
        mOutputNames[SEPERATE] = "seperate?";
    }
    struct __Init {
        __Init() {
            staticCtor();
        }
    };
    static map<Mode  ,string> mModeNames;
    static map<Method,string> mMethodNames;
    static map<Output,string> mOutputNames;
private:
    static __Init __init;
};
// NB Make sure the maps are defined before __init or the static
// constructor called by __init will try to use them before they are
// constructed.
map<EasySpinInput::Mode  ,string> EasySpinInput::mModeNames;
map<EasySpinInput::Method,string> EasySpinInput::mMethodNames;
map<EasySpinInput::Output,string> EasySpinInput::mOutputNames;
EasySpinInput::__Init EasySpinInput::__init;



//================================================================================//
// GUI Functions

void loadSpaceGroups();
vector<wxString> gSpaceGroups;

EasySpinFrame::EasySpinFrame(wxWindow* parent,
                             wxWindowID id,
                             const wxString& title,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style) 
    : EasySpinFrameBase(parent,id,title,pos,size,style) {
    loadSpaceGroups();

    //Add numeric validators to the numeric text boxes
    mCtrlMax->   SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));
    mCtrlMin->   SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));
    mCtrlCentre->SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));
    mCtrlSweep-> SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));

    mCtrlMWFreq->SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));
    mCtrlModAmp->SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));

    mCtrlNPoints->SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));

    SetMinMax(310,330);
    SetKnots(2);

    mCtrlMWFreq->ChangeValue(wxT("9.5"));

    mCtrlPhase->SetValidator(wxTextValidator(wxFILTER_NUMERIC,NULL));

    mOrientEditPanel = new OrientEditPanel(mPanelCystal);
    mSizerOrient->Add(mOrientEditPanel,1,wxEXPAND,2);
    Layout();
    mOrientEditPanel->Enable(false);

    //Options section
    mCtrlKnots->SetRange(2,INT_MAX);
    mCtrlAngularRes->sigUnFocus.connect(mem_fun(this,&EasySpinFrame::SlotAngularResUnFocus));
    mCtrlInterp->SetRange(2,INT_MAX);
}


void EasySpinFrame::OnMin(wxCommandEvent& e) {
    double min,max;
    mCtrlMax->GetValue().ToDouble(&max);
    mCtrlMin->GetValue().ToDouble(&min);

    if(min>max) {
        max = min;
        mCtrlMax->ChangeValue(wxString() << max);
    }

    mCtrlCentre->ChangeValue(wxString() << (min+max)/2);
    mCtrlSweep ->ChangeValue(wxString() << max-min);
}
void EasySpinFrame::OnMax(wxCommandEvent& e) {
    double min,max;
    mCtrlMax->GetValue().ToDouble(&max);
    mCtrlMin->GetValue().ToDouble(&min);

    if(min>max) {
        min = max;
        mCtrlMin->ChangeValue(wxString() << min);
    }

    mCtrlCentre->ChangeValue(wxString() << (min+max)/2);
    mCtrlSweep ->ChangeValue(wxString() << max-min);
}

void EasySpinFrame::OnCentreSweep(wxCommandEvent& e) {
    double centre,sweep;
    mCtrlCentre->GetValue().ToDouble(&centre);
    mCtrlSweep ->GetValue().ToDouble(&sweep);

    if(sweep < 0) {
        sweep = 0;
    }

    mCtrlMin->ChangeValue(wxString() << centre-sweep/2);
    mCtrlMax->ChangeValue(wxString() << centre+sweep/2);
}

void EasySpinFrame::OnXBand(wxCommandEvent& e) {
    SetMWFreq(9.5);
}

void EasySpinFrame::OnQBand(wxCommandEvent& e) {
    SetMWFreq(35);
}

void EasySpinFrame::OnWBand(wxCommandEvent& e) {
    SetMWFreq(95);
}

void EasySpinFrame::SetMWFreq(double freq) {
    mCtrlMWFreq->ChangeValue(wxString() << freq);
}

void EasySpinFrame::SetMinMax(double min,double max) {
    mCtrlMin->ChangeValue(wxString() << min);
    mCtrlMax->ChangeValue(wxString() << max);

    mCtrlCentre->ChangeValue(wxString() << (min+max)/2);
    mCtrlSweep ->ChangeValue(wxString() << max-min);
}

void EasySpinFrame::OnTempCheck(wxCommandEvent& e)  {
    bool checked = mCtrlTempOn->GetValue();
    mCtrlTemp->Enable(checked);
}

void EasySpinFrame::OnModAmpCheck(wxCommandEvent& e) {
    bool checked = mCtrlModAmpOn->GetValue();
    mCtrlModAmp->Enable(checked);
    mCtrlModAmpUnit->Enable(checked);
}

void EasySpinFrame::OnModAmpUnit(wxCommandEvent& e) {
    mCtrlRangeUnit->SetSelection(mCtrlModAmpUnit->GetSelection());
}

void EasySpinFrame::OnRangeUnit(wxCommandEvent& e) {
    mCtrlModAmpUnit->SetSelection(mCtrlRangeUnit->GetSelection());    
}

void EasySpinFrame::SetKnots(unsigned long nKnots) {
    mCtrlKnots->SetValue(nKnots);
    double angularRes = 90.0 / nKnots;
    mCtrlAngularRes->ChangeValue(wxString() << angularRes);
}

void EasySpinFrame::OnKnotsChange(wxSpinEvent& e) {
    SetKnots(mCtrlKnots->GetValue());
}

void EasySpinFrame::OnAngularResText(wxCommandEvent& e) {
}

void EasySpinFrame::SlotAngularResUnFocus() {
    double angularRes;
    mCtrlAngularRes->GetValue().ToDouble(&angularRes);

    //Find the closest angular resolution to angularRes that divides
    //90 degrees

    double fractionalKnots = 90/angularRes;
    unsigned long knots = round(fractionalKnots);
    SetKnots(knots);
}

void EasySpinFrame::OnInterpCheck(wxCommandEvent& e) {
    bool checked = mCtrlInterpOn->GetValue();
    mCtrlInterp->Enable(checked);
}

void EasySpinFrame::OnCystal(wxCommandEvent& e) {
    bool cystalChecked = mRadioCrystal->GetValue();
    mPanelCystal->Enable(cystalChecked);
    mCtrlSpaceGroupButton->Enable(cystalChecked);
    mCtrlSpaceGroupTxt->Enable(cystalChecked);
    mOrientEditPanel->Enable(cystalChecked);
}

void EasySpinFrame::RepopulateCrystalOrients() {
    mCtrlOrients->Clear();
    foreach(Orientation o,mOrients) {
        //mListCtrl->Append();
    }
}

void EasySpinFrame::OnAddOrient(wxCommandEvent& e) {
    mOrients.push_back(Orientation());
}

void EasySpinFrame::OnDeleteOrient(wxCommandEvent& e) {
    
}

void EasySpinFrame::OnShowSpaceGroups(wxCommandEvent& e) {
    //TODO: Memory leak?
    SpaceGroupDialog* spaceGroupDialog = new SpaceGroupDialog(this);
    spaceGroupDialog->ShowModal();
}

void EasySpinFrame::OnGenerate(wxCommandEvent& e) {
    EasySpinInput easySpinInput;

    //Collect these from the GUI
    easySpinInput.setCentreSweep(300,20);
    easySpinInput.setMWFreq(9.5);
    easySpinInput.setTemperature(70);
    easySpinInput.setNPoints(1024);
    easySpinInput.setHarmonic(1);
    easySpinInput.setMode(EasySpinInput::PARALLEL);
    easySpinInput.setSpaceGroup(1);
    easySpinInput.setMWPhase(0);
    easySpinInput.setMethod(EasySpinInput::PERT1);
    easySpinInput.setNKnots(8);
    easySpinInput.setInterpolate(3);
    easySpinInput.setOutput(EasySpinInput::SUMMED);

    //Do code generation
    string easyCode = easySpinInput.generate();
    
    mCtrlPreview->ChangeValue(wxString(easyCode.c_str(),wxConvUTF8));
}


BEGIN_EVENT_TABLE(EasySpinFrame,wxFrame)

//Experiment
EVT_TEXT(ID_MAX,   EasySpinFrame::OnMax)
EVT_TEXT(ID_MIN,   EasySpinFrame::OnMin)
EVT_TEXT(ID_CENTRE,EasySpinFrame::OnCentreSweep)
EVT_TEXT(ID_SWEEP, EasySpinFrame::OnCentreSweep)

EVT_BUTTON(ID_XBAND,EasySpinFrame::OnXBand)
EVT_BUTTON(ID_QBAND,EasySpinFrame::OnQBand)
EVT_BUTTON(ID_WBAND,EasySpinFrame::OnWBand)

EVT_CHECKBOX(ID_TEMPON,EasySpinFrame::OnTempCheck)
EVT_CHECKBOX(ID_MODAMPON,EasySpinFrame::OnModAmpCheck)

EVT_CHOICE(ID_MODAMPUNIT,EasySpinFrame::OnModAmpUnit)
EVT_CHOICE(ID_RANGEUNIT ,EasySpinFrame::OnRangeUnit)

EVT_RADIOBUTTON(ID_CRYSTAL,EasySpinFrame::OnCystal)
EVT_RADIOBUTTON(ID_POWDER,EasySpinFrame::OnCystal)

EVT_BUTTON(ID_ADD_ORIENT,   EasySpinFrame::OnAddOrient)
EVT_BUTTON(ID_DELETE_ORIENT,EasySpinFrame::OnDeleteOrient)

EVT_BUTTON(ID_SPACEGROUP_BUTTON,EasySpinFrame::OnShowSpaceGroups)

//Options
EVT_SPINCTRL(ID_KNOTS,      EasySpinFrame::OnKnotsChange)

EVT_CHECKBOX(ID_INTERPON,EasySpinFrame::OnInterpCheck)

//Other
EVT_BUTTON(ID_TMP_GEN, EasySpinFrame::OnGenerate)

END_EVENT_TABLE()


void loadSpaceGroups() {
    static bool loaded = false;
    if(loaded) {
        return;
    }

    fstream fin("data/spacegroups.txt",ios::in);
    if(!fin.is_open()) {
        wxLogError(wxT("Could not open data/spacegroups.txt, no spacegroups will be available\n"));
        return;
    }

    long count = 0;
    while(!fin.eof()) {
        //TODO: I'm not sure if this code will work on windows, where
        //wstrings need to be used rather than strings
        string symbol;
        fin >> symbol;
        if(symbol == "") {
            //Ignore blank lines
            continue;
        }
        gSpaceGroups.push_back(wxString(symbol.c_str(),wxConvUTF8));
        count++;
    }
    if(count != 230) {
        wxLogError(wxString() << wxT("Expecting 230 entries in data/spacegroups.txt, found\n") << count);
        return;
    }

    loaded = true;
    return;
}
