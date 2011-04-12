
#include <gui/SpinInteractionEdit.hpp>
#include <gui/SpinachApp.hpp>
#include <shared/foreach.hpp>

using namespace SpinXML;

//============================================================//
// SpinInterEditPanel

SpinInterEditPanel::SpinInterEditPanel(wxWindow* parent,wxWindowID id) 
	: SpinInterEditPanelBase(parent,id),
	  mEditMode(EDIT_ALL),
	  mSpin(SpinView(NULL,GetFrame(),GetUnitSystem())),
	  mUpdatingListBox(false) {
	mInterEdit=new InterEditPanel(this);
	GetSizer()->Add(mInterEdit,1,wxEXPAND | wxALL);
	Enable(false);
};

SpinInterEditPanel::~SpinInterEditPanel() {
	Clear();
}

void SpinInterEditPanel::SetSpin(Spin* spin) {
	mSpin=SpinView(spin,GetFrame(),GetUnitSystem());
	LoadFromSpin();
}

void SpinInterEditPanel::Clear() {
	Enable(false);
}

void SpinInterEditPanel::OnNewButton(wxCommandEvent& e) {
	Interaction* inter=new Interaction(0.0*Hz,Interaction::SHIELDING,mSpin.Get());
	GetRawSS()->InsertInteraction(inter);

	UpdateListBox();
	mInterListBox->SetSelection(mInterListBox->GetCount()-1);
	mInterEdit->SetInter(inter,mSpin.Get());
	InteractionChange();
}

void SpinInterEditPanel::OnDeleteButton(wxCommandEvent& e) {
	long n=mInterListBox->GetSelection();
	GetRawSS()->DiscardInteraction((Interaction*)mInterListBox->GetClientData(n));
	UpdateListBox();
	mInterEdit->SetInter(NULL,NULL);
	InteractionChange();
}

void SpinInterEditPanel::LoadFromSpin() {
	Clear();
	if(mSpin.Get()==NULL) {
		return;
	} else {
		Enable(true);
	}

	UpdateListBox();
	InteractionChange();
}

void SpinInterEditPanel::UpdateListBox() {
	mUpdatingListBox=true;
	mInterListBox->Clear();

	std::vector<Interaction*> inters = GetRawSS()->GetInteractionsBySpin(mSpin.Get());

	foreach(Interaction* inter,inters) {
		if(inter->GetIsQuadratic()) {
			if(mEditMode!=EDIT_QUAD && mEditMode!=EDIT_ALL) {
				continue;
			}
		} else if (inter->GetIsBilinear()) {
			if(mEditMode!=EDIT_BILINEAR && mEditMode!=EDIT_ALL) {
				continue;
			}
		} else {
			if(mEditMode!=EDIT_LINEAR && mEditMode!=EDIT_ALL) {
				continue;
			}
		}
		wxString interTitle=NameInteraction(inter);
		mInterListBox->Append(interTitle,(void*)inter);
	}
	mUpdatingListBox=false;
}

wxString SpinInterEditPanel::NameInteraction(Interaction* inter) {
	wxString interTitle;
	interTitle << wxString(Interaction::GetTypeName(inter->GetType()),wxConvUTF8);
	interTitle << wxT("(");
	interTitle << wxString(Interaction::GetStorageName(inter->GetStorage()),wxConvUTF8);
	interTitle << wxT(")");

	return interTitle;
}
       
void SpinInterEditPanel::OnInteractionChange(wxCommandEvent& e) {
	if(mUpdatingListBox) {
		return;
	}
	InteractionChange();
}

void SpinInterEditPanel::InteractionChange() {
	long selected=mInterListBox->GetSelection();
	if(selected >-1) {
		Interaction* inter = (Interaction*)mInterListBox->GetClientData(selected);
		mInterEdit->SetInter(inter,mSpin.Get());
	} else {
		mInterEdit->SetInter(NULL,NULL);
	}
}


BEGIN_EVENT_TABLE(SpinInterEditPanel,wxPanel)

EVT_BUTTON     (INTER_ADD   ,SpinInterEditPanel::OnNewButton)
EVT_BUTTON     (INTER_DELETE,SpinInterEditPanel::OnDeleteButton)
EVT_LISTBOX    (wxID_ANY,SpinInterEditPanel::OnInteractionChange)

END_EVENT_TABLE()
