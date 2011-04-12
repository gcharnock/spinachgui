#include <gui/RootFrame.hpp>

#include <gui/InterDisplaySettings.hpp>
#include <gui/RightClickMenu.hpp>
#include <3d/glgeometry.hpp>
#include <3d/displaySettings.hpp>
#include <gui/Display3D.hpp>
#include <gui/SpinachApp.hpp>
#include <gui/MolSceneGraph.hpp>
#include <gui/SpinGrid.hpp>
#include <gui/SpinInteractionEdit.hpp>

#include <shared/spinsys.hpp>

#include <stdexcept>
#include <wx/log.h>
#include <wx/statusbr.h>
#include <wx/treectrl.h>
#include <wx/aui/aui.h>
#include <shared/foreach.hpp>

//Input and output filters
#define ID_UNIT_START 12345


using namespace std;
using namespace SpinXML;
using sigc::bind;
using sigc::mem_fun;
//============================================================//
// Utility Functions

wxString GetExtension(const wxString& filename) {
    int dot=filename.find(wxT("."));
    wxString ext=(dot != wxNOT_FOUND ? filename.Mid(dot+1) : wxT(""));
    return ext;
}

//============================================================//
// Our spin system display

class SpinSysDisplay3D : public Display3D {
public:
	SpinSysDisplay3D(wxWindow* parent) 
		: Display3D(parent) {
		GetSS().sigReloaded.connect(mem_fun(this,&Display3D::ResetView));
	}
protected:
	virtual void DrawScene() {
		lighting.On();

		mMolScene.Draw();

		translucent.On();
		mInteractionScene.Draw();
		translucent.Off();

		//Also draw the interactions as wireframes
		wire.On();
		mInteractionScene.Draw();
		wire.Off();

		lighting.Off();

		StartPicking();
		mMolScene.Draw();
		StopPicking();
	}
	virtual void OnMouseOver3D(int stackLength,const GLuint* ClosestName) {
		if(stackLength == 0) {
			SetHover(NULL);
			return;
		}
		if(ClosestName[0] == NAME_SPINS) {
			Spin* hover = GetHover();
			Spin* newHover = GetRawSS()->GetSpin(ClosestName[stackLength-1]);
			if(hover != newHover) {
				SetHover(newHover);
			}
		} else {
			SetHover(NULL);
		}
	}
	
private:
	//These nodes can be rotated and translated  with the mouse
	static GLLighting lighting;
	static GLTranslucent translucent;
	static GLWire wire;

    SpinSysScene     mMolScene;
    InteractionScene mInteractionScene;
};
GLLighting    SpinSysDisplay3D::lighting;
GLTranslucent SpinSysDisplay3D::translucent;
GLWire        SpinSysDisplay3D::wire;


//============================================================//
// Reference frame tree view

class RCActionActivateFrame : public RightClickAction {
public:
	RCActionActivateFrame(Frame* frame) 
		: RightClickAction(wxT("Activate Frame")), mFrame(frame) {
	}
	bool Visible() const {return true;}
	void Exec(wxCommandEvent& e) {
		SetFrame(mFrame);
	}
private:
	Frame* mFrame;
};

//Quick class working with the wxWidgets clientData system
struct FramePointer : public wxTreeItemData {
    FramePointer(Frame* frame)
	: frame(frame) {
    }
    Frame* frame;
};

//Class for drawing the tree of reference frames.
class FrameTree : public wxTreeCtrl , public sigc::trackable {
public:

	FrameTree(wxWindow* parent) : wxTreeCtrl(parent) {
		mRoot = AddRoot(wxT("Lab Frame"),-1,-1,new FramePointer(GetRawSS()->GetLabFrame()));


		RefreshFromSpinSystem();
		sigFrameChange.connect(mem_fun(this,&FrameTree::SlotFrameChange));
	}
	
	void RefreshFromSpinSystem() {
		mapFrameToId.clear();
		mapFrameToId[GetSS().GetLabFrame()] = mRoot;

		RefreshFromSpinSystemRecursive(mRoot,GetRawSS()->GetLabFrame());

		mActive = mapFrameToId[GetFrame()];
		SetItemBold(mActive);
		Refresh(); //Seems like we need to explicitly ask for a
				   //repaint
	}

	void SlotFrameChange(Frame* frame) {
		SetItemBold(mActive,false);
		mActive = mapFrameToId[frame];
		SetItemBold(mActive);
		Refresh();
	}
private:
	void RefreshFromSpinSystemRecursive(wxTreeItemId itemId,Frame* frame) {
		mapFrameToId[frame] = itemId;
		vector<Frame*> children = frame->GetChildren();
		for(vector<Frame*>::iterator i = children.begin();i != children.end();++i) {
			wxTreeItemId nextItemId = AppendItem(itemId,wxT("Frame"),-1,-1,new FramePointer(*i));
			RefreshFromSpinSystemRecursive(nextItemId,*i);
		}
	}
    
    void OnRightClick(wxTreeEvent& e) {
		FramePointer* fp = (FramePointer*) GetItemData(e.GetItem());
		RightClickMenu* menu = new RightClickMenu(this);

		vector<RightClickAction*> actions;
		actions.push_back(new RCActionActivateFrame(fp->frame));

		menu->Build(actions);
		PopupMenu(menu);
		delete menu;
    }
    DECLARE_EVENT_TABLE();
	wxTreeItemId mRoot;
	wxTreeItemId mActive;
	map<Frame*,wxTreeItemId> mapFrameToId;
};


BEGIN_EVENT_TABLE(FrameTree,wxTreeCtrl)

EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, FrameTree::OnRightClick)

END_EVENT_TABLE()


//============================================================//
// Custom Status Bar

class StatusBar : public wxStatusBar {
public:
    StatusBar(wxWindow* parent) : wxStatusBar(parent) {
	int widths_field[] = {-1,80,80};
	SetFieldsCount(3,widths_field);
	SlotUnitChange(DIM_LENGTH,GetUnit(DIM_LENGTH));
	SlotUnitChange(DIM_ENERGY,GetUnit(DIM_ENERGY));
    }

    void SlotUnitChange(PhysDimension d,unit u) {
	wxString str = wxString(u.get_name().c_str(),wxConvUTF8);
	switch(d) {
	case DIM_LENGTH:
	    SetStatusText(str,1);
	    break;
	case DIM_ENERGY:
	    SetStatusText(str,2);
	    break;
	}
    }

    void SlotSizeChange(Interaction::Type t,double s) {

    }
};

//============================================================//

class InterDisplaySettingsPanel : public wxPanel,public sigc::trackable {
public:
	InterDisplaySettingsPanel(wxWindow* parent) : wxPanel(parent) {
		wxBoxSizer* bs=new wxBoxSizer(wxVERTICAL);

		//HACK: Quick hack to iterate though an enum
		for(int i = Interaction::HFC;i != Interaction::TYPE_END;++i) {
			Interaction::Type type = (Interaction::Type)i;
			InterDisplaySettings* widget = new InterDisplaySettings(this,type);
			bs->Add(widget,1,wxEXPAND);
		}

		this->SetSizer(bs);
	}
};

//============================================================//
// RootFrame

void RootFrame::InitFrame() {
	//Setup the status bar
	StatusBar* statusBar = new StatusBar(this);
	SetStatusBar(statusBar);

	//Set up the AUI, including the view menu function
    mAuiManager=new wxAuiManager(this);

    mInterSizePanel= new InterDisplaySettingsPanel(this);
	mSpinGrid      = new SpinGrid(this);
	mSpinInterEdit = new SpinInterEditPanel(this);
	mDisplay3D     = new SpinSysDisplay3D(this);
	mFrameTree     = new FrameTree(this);

    // add the panes to the manager
    wxAuiPaneInfo display3dinfo;
    display3dinfo.Center();
    display3dinfo.CaptionVisible(false);
    display3dinfo.CloseButton(false);
    display3dinfo.Movable(false);
    mAuiManager->AddPane(mDisplay3D,display3dinfo);
    mAuiManager->AddPane(mSpinGrid,wxBOTTOM,wxT("Spins"));
	mAuiManager->AddPane(mFrameTree,wxRIGHT,wxT("Reference Frames"));

    wxAuiPaneInfo tensorVisinfo;
	tensorVisinfo.Float();
	tensorVisinfo.Hide();
	tensorVisinfo.Caption(wxT("Tensor Visualisation"));
    mAuiManager->AddPane(mInterSizePanel,tensorVisinfo);
    mAuiManager->AddPane(mSpinInterEdit,wxBOTTOM,wxT("Interaction Editor"));

    //Grey the undo and redo menu ideams. They can be ungreyed when
    //there is an undo history

    mMenuItemUndo->Enable(false);
    mMenuItemRedo->Enable(false);

    //Connect up the signals
    mSpinGrid->sigSelect.connect(mem_fun(mSpinInterEdit,&SpinInterEditPanel::SetSpin));
	sigUnitChange.connect(mem_fun(statusBar,&StatusBar::SlotUnitChange));

	//Units menu. To avoid writing an On* function for every unit
	//(which would make making units configurable impossible) we
	//connect them all to the same handler and setup a lookup for
	//translating the id into a unit and physical dimension.
	typedef pair<PhysDimension,unit> p;

	mIdToUnit.push_back(p(DIM_LENGTH,Angstroms));  //Default
	mIdToUnit.push_back(p(DIM_LENGTH,nanometre));
	mIdToUnit.push_back(p(DIM_LENGTH,metres));

	mIdToUnit.push_back(p(DIM_ENERGY,Hz));  //Default
	mIdToUnit.push_back(p(DIM_ENERGY,KHz));
	mIdToUnit.push_back(p(DIM_ENERGY,MHz));
	mIdToUnit.push_back(p(DIM_ENERGY,eV));
	mIdToUnit.push_back(p(DIM_ENERGY,Joules));

	for(unsigned long i = 0;i<mIdToUnit.size();i++) {
		PhysDimension d = mIdToUnit[i].first;
		unit u = mIdToUnit[i].second;

		wxMenu *menu = d == DIM_LENGTH ? mMenuLength : mMenuEnergy;
		menu->AppendRadioItem(ID_UNIT_START+i,wxString(u.get_name().c_str(),wxConvUTF8));
	}
	wxObjectEventFunction afterCast = 
		(wxObjectEventFunction)(wxEventFunction)(&RootFrame::OnUnitChange);
	Connect(ID_UNIT_START,ID_UNIT_START+mIdToUnit.size(),wxEVT_COMMAND_MENU_SELECTED,afterCast);

}


void RootFrame::OnUnitChange(wxCommandEvent& e) {
	pair<PhysDimension,unit> thePair = mIdToUnit[e.GetId()-ID_UNIT_START];
	PhysDimension d = thePair.first;
	unit u = thePair.second;
	SetUnit(d,u);
}

void RootFrame::OnUndo(wxCommandEvent& e) {

}

void RootFrame::OnRedo(wxCommandEvent& e) {

}

void RootFrame::OnNew(wxCommandEvent& e) {
    GetRawSS()->Clear();
    mOpenPath=wxT("Untitled");
    mOpenFile=wxT("Untitled");
    mOpenDir=wxT("");
    UpdateTitle();
}

void RootFrame::OnSupress(wxCommandEvent& e) {
    SetSupressedSpins(GetSelection());
}

void RootFrame::OnUnSupress(wxCommandEvent& e) {
    set<Spin*> empty; //Empty set
    SetSupressedSpins(empty);
}


void RootFrame::UpdateTitle() {
    SetTitle(wxString() << mOpenFile << wxT(" - Spinach (") << mOpenPath << wxT(")"));
}

void RootFrame::OnOpen(wxCommandEvent& e) {
    wxString filter;
    bool first=true;
    for(vector<ISpinSystemLoader*>::const_iterator i=wxGetApp().GetIOFilters().begin();
        i!=wxGetApp().GetIOFilters().end();
        ++i) {
        if((*i)->GetFilterType()==ISpinSystemLoader::LOAD ||
           (*i)->GetFilterType()==ISpinSystemLoader::LOADSAVE) {
            if(!first) {
                filter << wxT("|");
            } else {
                first=false;
            }
            wxString ThisFilter((*i)->GetFilter(),wxConvUTF8);
            wxString ThisFilterName((*i)->GetFormatName(),wxConvUTF8);
            filter << ThisFilterName << wxT(" (*.") << ThisFilter << wxT(")|*.") << ThisFilter;
        }
    }
    wxFileDialog* fd=new wxFileDialog(this,
                                      wxString(wxT("Choose a file")),
                                      wxString(wxT("")), //Default file
                                      wxString(wxT("")), //Default dir
                                      wxString(filter) ,
                                      wxFD_OPEN);
    if(fd->ShowModal() == wxID_OK) {
        LoadFromFile(mOpenPath=fd->GetPath(),mOpenDir=fd->GetDirectory(),fd->GetFilename());
    }

}

void RootFrame::LoadFromFile(const wxString& path,const wxString& dir, const wxString& filename) {
    mOpenPath=path;
    mOpenFile=filename;
    mOpenDir=dir;
    wxString ext=GetExtension(mOpenFile).Lower();

    ISpinSystemLoader* loader=NULL;
    typedef vector<ISpinSystemLoader*>::const_iterator itor;
    for(itor i=wxGetApp().GetIOFilters().begin();
        i!=wxGetApp().GetIOFilters().end();
        ++i) {
        if((*i)->GetFilterType()==ISpinSystemLoader::LOAD ||
           (*i)->GetFilterType()==ISpinSystemLoader::LOADSAVE) {
            if(ext==wxString((*i)->GetFilter(),wxConvUTF8).Lower()) {
                loader=*i;
                break;
            }
        }
    }
    if(loader==NULL) {
        wxLogError(wxString() << wxT("Unknown extension ") << ext);
    } else {
        try {
            GetRawSS()->LoadFromFile(mOpenPath.char_str(),loader);
        } catch(const runtime_error& e) {
            wxString msg(e.what(),wxConvUTF8);
            wxLogError(wxString() << wxT("File is corrupt:\n") << msg);

        }
    }
    Chkpoint(wxT("Load File"));
    UpdateTitle();
}

void RootFrame::OnSave(wxCommandEvent& e) {
    if(GetExtension(mOpenFile) == wxT("xml")) {
        SaveToFile(mOpenPath);
    } else {
        SaveAs();
    }
}

void RootFrame::OnSaveAs(wxCommandEvent& e) {
    SaveAs();
}

void RootFrame::SaveAs() {
    wxString filter;
    bool first=true;
    for(vector<ISpinSystemLoader*>::const_iterator i=wxGetApp().GetIOFilters().begin();
        i!=wxGetApp().GetIOFilters().end();
        ++i) {
        if((*i)->GetFilterType()==ISpinSystemLoader::SAVE ||
           (*i)->GetFilterType()==ISpinSystemLoader::LOADSAVE) {
            if(!first) {
                filter << wxT("|");
            } else {
                first=false;
            }
            wxString ThisFilter((*i)->GetFilter(),wxConvUTF8);
            wxString ThisFilterName((*i)->GetFormatName(),wxConvUTF8);
            filter << ThisFilterName << wxT(" (*.") << ThisFilter << wxT(")|*.") << ThisFilter;
        } 
    }
    wxFileDialog* fd=new wxFileDialog(this,
                                      wxString(wxT("Choose a file")),
                                      wxString(wxT("")), //Default file
                                      wxString(wxT("")), //Default dir
                                      filter,
                                      wxFD_SAVE);
    if(fd->ShowModal() == wxID_OK) {
        mOpenPath=fd->GetPath();
        mOpenFile=fd->GetFilename();
        mOpenDir=fd->GetDirectory();
        wxString ext=GetExtension(mOpenFile).Lower();
        ISpinSystemLoader* saver=NULL;
        for(vector<ISpinSystemLoader*>::const_iterator i=wxGetApp().GetIOFilters().begin();
            i!=wxGetApp().GetIOFilters().end();
            ++i) {
            if((*i)->GetFilterType()==ISpinSystemLoader::SAVE ||(*i)->GetFilterType()==ISpinSystemLoader::LOADSAVE) {
                if(ext==wxString((*i)->GetFilter(),wxConvUTF8).Lower()) {
                    saver=*i;
                    break;
                }
            }
        }
        if(saver==NULL) {
            wxLogError(wxString() << wxT("Unknown extension ") << ext);
        } else {
            GetRawSS()->SaveToFile(mOpenPath.char_str(),saver);
        }
        UpdateTitle();
    }
}

void RootFrame::OnExit(wxCommandEvent& e) {
    delete this;
}

void RootFrame::SaveToFile(const wxString& filename,ISpinSystemLoader* saver) {
    if(saver==NULL) {
        saver=mSaver;
    }
    GetRawSS()->SaveToFile(filename.char_str(),saver);
    mSaver=saver;
}

void RootFrame::OnNmrEpr(wxCommandEvent& e) {
    mMenuItemNmrEpr->Check(true);
    mRootToolbar->ToggleTool(ID_NMR_EPR,true);
}

void RootFrame::OnNmr(wxCommandEvent& e) {
    mMenuItemNmr->Check(true);
    mRootToolbar->ToggleTool(ID_NMR,true);
}

void RootFrame::OnEpr(wxCommandEvent& e) {
    mMenuItemEpr->Check(true);
    mRootToolbar->ToggleTool(ID_EPR,true);
}


void RootFrame::OnResize(wxSizeEvent&e) {
    mAuiManager->Update();
}

void RootFrame::OnBondToggle(wxCommandEvent& e) {
    bool showBonds=e.IsChecked();
    mMenuItemBondToggle->Check(showBonds);
    mRootToolbar->ToggleTool(ID_BOND_TOGGLE,showBonds);
    SetShowBonds(showBonds);
}
	     
void RootFrame::OnToggle3D(wxCommandEvent& e) {
	AUIToggle(mDisplay3D);
}

void RootFrame::OnToggleGrid(wxCommandEvent& e) {
	AUIToggle(mSpinGrid);
}

void RootFrame::OnToggleTensorVis(wxCommandEvent& e) {
	AUIToggle(mInterSizePanel);
}

void RootFrame::OnToggleInterEdit(wxCommandEvent& e) {
	AUIToggle(mSpinInterEdit);
}

void RootFrame::OnToggleFrames(wxCommandEvent& e) {
	AUIToggle(mFrameTree);
}

void RootFrame::AUIToggle(wxWindow* p) {
	cout << "AUI Toggle" << endl;
    wxAuiPaneInfo& info = mAuiManager->GetPane(p);
	info.Show(!info.IsShown());
	mAuiManager->Update();
}

void RootFrame::OnMakeIso(wxCommandEvent& e) {
    //cleanup: put this algorithum somewhere more logical
    
    set<Spin*> selection = GetSelection();

    foreach(Spin* spin,selection) {
	vector<Interaction*> inters = GetRawSS()->GetInteractionsBySpin(spin);

	foreach(Interaction* inter,inters) {
	    inter->ToScalar();
	}
    }
}

void RootFrame::OnCarbon(wxCommandEvent& e) {
    ClearSelection();
    vector<Spin*> spins = GetRawSS()->GetSpins();
    foreach(Spin* spin,spins) {
	if(spin->GetElement() == 6) {
	    AddSelection(spin);
	}
    }
}

void RootFrame::OnHydrogen(wxCommandEvent& e) {
    ClearSelection();
    vector<Spin*> spins = GetRawSS()->GetSpins();
    foreach(Spin* spin,spins) {
	if(spin->GetElement() == 1) {
	    AddSelection(spin);
	}
    }
}


BEGIN_EVENT_TABLE(RootFrame,wxFrame)

//File Menu
EVT_MENU(ID_NEW   ,RootFrame::OnNew   )
EVT_MENU(ID_OPEN  ,RootFrame::OnOpen  )
EVT_MENU(ID_SAVE  ,RootFrame::OnSave  )
EVT_MENU(ID_SAVEAS,RootFrame::OnSaveAs)
EVT_MENU(ID_EXIT  ,RootFrame::OnExit  )

//Edit Menu
EVT_MENU(ID_UNDO,RootFrame::OnUndo)
EVT_MENU(ID_REDO,RootFrame::OnRedo)

EVT_MENU(ID_EDIT_ISO,RootFrame::OnMakeIso)

//View Menu
EVT_MENU(ID_NMR_EPR,RootFrame::OnNmrEpr)
EVT_MENU(ID_NMR,    RootFrame::OnNmr)
EVT_MENU(ID_EPR,    RootFrame::OnEpr)

EVT_MENU(ID_BOND_TOGGLE,  RootFrame::OnBondToggle)

EVT_MENU(ID_VIEW_3D,        RootFrame::OnToggle3D)
EVT_MENU(ID_VIEW_GRID,      RootFrame::OnToggleGrid)
EVT_MENU(ID_VIEW_TENSORVIS, RootFrame::OnToggleTensorVis)
EVT_MENU(ID_VIEW_TENSORVIS, RootFrame::OnToggleInterEdit)
EVT_MENU(ID_VIEW_FRAMES,    RootFrame::OnToggleFrames)

EVT_MENU(ID_SUPRESS_SELECTION,RootFrame::OnSupress)
EVT_MENU(ID_UNSUPRESS        ,RootFrame::OnUnSupress)

//Selection Menu
EVT_MENU(ID_SEL_HYDROGEN, RootFrame::OnHydrogen)
EVT_MENU(ID_SEL_CARBON  , RootFrame::OnCarbon)

//Resize
EVT_SIZE(RootFrame::OnResize)

EVT_ERASE_BACKGROUND(RootFrame::OnEraseBackground)

END_EVENT_TABLE()


