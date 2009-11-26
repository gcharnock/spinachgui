
#include <gui/RootFrame.hpp>
#include <gui/SpinachApp.hpp>
#include <gui/StdEvents.hpp>
#include <stdexcept>
#include <wx/log.h>

//Input and output filters
#include <shared/formats/xyz.hpp>

using namespace std;

//============================================================//
// Utility Functions

wxString GetExtension(const wxString& filename) {
  int dot=filename.find(wxT("."));
  wxString ext=(dot != wxNOT_FOUND ? filename.Mid(dot+1) : wxT(""));
  return ext;
}

//============================================================//
// RootFrame

void RootFrame::InitFrame() {
  mNotebook=new wxAuiNotebook(mAuiPanel);

  mSpinGridPanel=new SpinGridPanel(mNotebook);

  mGLDisplay=new glDisplay(mNotebook);

  // add the panes to the manager
  mNotebook->AddPage(mGLDisplay, wxT("3D View"));
  mNotebook->AddPage(mSpinGridPanel, wxT("Grid View"));

  mAuiPanel->GetSizer()->Add(mNotebook,1,wxEXPAND);

  //Grey the undo and redo menu ideams. They can be ungreyed when
  //there is an undo history

  mMenuItemUndo->Enable(false);
  mMenuItemRedo->Enable(false);
}


void RootFrame::OnUndo(wxCommandEvent& e) {
  wxGetApp().GetSpinSysManager()->Undo();
}

void RootFrame::OnRedo(wxCommandEvent& e) {
  wxGetApp().GetSpinSysManager()->Redo();
}

void RootFrame::OnNew(wxCommandEvent& e) {
  GetSS()->Clear();
  mOpenPath=wxT("Untitled");
  mOpenFile=wxT("Untitled");
  mOpenDir=wxT("");
  UpdateTitle();
}

void RootFrame::UpdateTitle() {
  SetTitle(wxString() << mOpenFile << wxT(" - Spinach (") << mOpenPath << wxT(")"));
}

void RootFrame::OnOpen(wxCommandEvent& e) {
  wxString filter;
  filter << wxT("Spin XML files (*.xml)|*.xml");
  filter << wxT("|Gausian03 files (*.log)|*.log");
  filter << wxT("|XYZ files (*.xyz)|*.xyz");
  filter << wxT("|All Files (*.*)|*.*");
  wxFileDialog* fd=new wxFileDialog(this,
				    wxString(wxT("Choose a file")),
				    wxString(wxT("")), //Default file
				    wxString(wxT("")), //Default dir
				    wxString(filter) ,
				    wxFD_OPEN);
  if(fd->ShowModal() == wxID_OK) {
    mOpenPath=fd->GetPath();
    mOpenFile=fd->GetFilename();
    mOpenDir=fd->GetDirectory();
    wxString ext=GetExtension(mOpenFile);

    if(ext.Lower()==wxT("xyz")) {
      try {
	XYZLoader* loader=new XYZLoader;
	GetSS()->LoadFromFile(mOpenPath.char_str(),loader);
	delete loader;
      } catch(const runtime_error& e) {
	wxLogError(wxT("Error Parsing XYZ file. File is corrupt"));
	wxLogError(wxString(e.what(),wxConvUTF8));
      }
    } else if(ext.Lower()==wxT("log")) {
      try {
	GetSS()->LoadFromG03File(mOpenPath.char_str());
      } catch(const runtime_error& e) {
	wxLogError(wxT("Error Parsing log file. File is corrupt"));
	wxLogError(wxString(e.what(),wxConvUTF8));
      }
    } else {
      //assume xml for everything else
      try {
	GetSS()->LoadFromXMLFile(mOpenPath.char_str());
      } catch(const runtime_error& e) {
	wxLogError(wxT("Error Parsing XML file. File is corrupt"));
	wxLogError(wxString(e.what(),wxConvUTF8));
      }
    }
    Chkpoint(wxT("Load File"));
    UpdateTitle();
    GetSSMgr().ClearHistory();
  }
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
  filter << wxT("Spin XML files (*.xml)|*.xml");
  filter << wxT("|XYZ Files (*.xyz)|*.xyz");
  filter << wxT("|All Files (*.*)|*.*");
  wxFileDialog* fd=new wxFileDialog(this,
				    wxString(wxT("Choose a file")),
				    wxString(wxT("")), //Default file
				    wxString(wxT("")), //Default dir
				    filter ,
				    wxFD_SAVE);
  if(fd->ShowModal()) {
    mOpenPath=fd->GetPath();
    mOpenFile=fd->GetFilename();
    mOpenDir=fd->GetDirectory();
    wxString ext=GetExtension(mOpenFile);
    if(ext.Lower()==wxT("xyz")) {
      SaveToFile(fd->GetPath(),XYZ_FILE);
    } else {
      //assume xml for everything else
      SaveToFile(fd->GetPath(),XML_FILE);
    }

    SetTitle(wxString() << mOpenFile << wxT(" - Spinach (") << mOpenPath << wxT(")"));
  }
}

void RootFrame::OnExit(wxCommandEvent& e) {
  delete this;
}

void RootFrame::SaveToFile(const wxString& filename,FileType ft) {
  if(ft==DEFAULT_FILE) {
    ft=mFt;
  }
  if(ft==XML_FILE) {
    GetSS()->SaveToXMLFile(filename.char_str());
  } else if(ft==XYZ_FILE) {
    XYZLoader* saver=new XYZLoader;
    GetSS()->SaveToFile(filename.char_str(),saver);
    delete saver;
  } else {
    throw runtime_error("RootFrame::SaveToFile - Unknown file type");
  }
  mFt=ft;
}

void RootFrame::OnNmrEpr(wxCommandEvent& e) {
  mGLDisplay->SetMode(glDisplay::NMR_EPR);
  mMenuItemNmrEpr->Check(true);
  mRootToolbar->ToggleTool(ID_NMR_EPR,true);
}

void RootFrame::OnNmr(wxCommandEvent& e) {
  mGLDisplay->SetMode(glDisplay::NMR);
  mMenuItemNmr->Check(true);
  mRootToolbar->ToggleTool(ID_NMR,true);
}

void RootFrame::OnEpr(wxCommandEvent& e) {
  mGLDisplay->SetMode(glDisplay::EPR);
  mMenuItemEpr->Check(true);
  mRootToolbar->ToggleTool(ID_EPR,true);
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

//View Menu
EVT_MENU(ID_NMR_EPR,RootFrame::OnNmrEpr)
EVT_MENU(ID_NMR,    RootFrame::OnNmr)
EVT_MENU(ID_EPR,    RootFrame::OnEpr)

END_EVENT_TABLE()


