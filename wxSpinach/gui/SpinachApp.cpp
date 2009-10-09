
#include <gui/SpinachApp.hpp>

#include <gui/SpinDialog.hpp>

#include <shared/nuclear_data.hpp>
#include <wx/log.h>

IMPLEMENT_APP(SpinachApp);

bool SpinachApp::OnInit() {

  try {
    LoadIsotopes();
  } catch(runtime_error e) {
    cout << "Isotopes not loaded" << endl;
    wxLogError(wxString() <<
	       wxT("Error loading data/isotopes.dat. Is the file present and not corrupt?\n") << 
	       wxT("Message was:") << 
	       wxString(e.what(),wxConvUTF8));
    return false;
  }


  mSS = shared_ptr<SpinSystem>(new SpinSystem);
  mSS->LoadFromG03File("data/tryosine.log");
  mSS->SaveToXMLFile("data/tryosine.xml");

  mSSMgr = shared_ptr<SpinSysManager>(new SpinSysManager(shared_ptr<SpinSystem>(new SpinSystem)));
  const SpinSysPtr* head = mSSMgr->Get();

  mSSMgr->Checkpoint();
  (*head)->LoadFromG03File("install/data/tryosine.log");

  RootFrame* frame = new RootFrame(NULL);

  //SpinDialog* dialog = new SpinDialog(frame,mSS->GetSpin(0));
  //dialog->ShowModal();

  frame->Show();
  return true;
}
