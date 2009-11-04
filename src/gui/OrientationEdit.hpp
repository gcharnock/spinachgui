
#ifndef ORIENTEDIT_H
#define ORIENTEDIT_H

#include <res/SpinachGUI.h>
#include <shared/spinsys.hpp>
#include <gui/DialogCombo.hpp>

//============================================================//
// OrientEditPanel

class OrientEditPanel : public OrientEditPanelBase {
public:
  OrientEditPanel(wxWindow* parent,
		  wxWindowID id = wxID_ANY,
		  const wxPoint& pos = wxDefaultPosition,
		  const wxSize& size = wxDefaultSize,
		  long style = wxTAB_TRAVERSAL);

  void SetOrient(const SpinXML::Orientation& orient);
  const SpinXML::Orientation& GetOrient() {return mOrient;}  

  void OnTextChange(wxCommandEvent& e);
  void OnPageChange(wxChoicebookEvent& e);

  void LoadFromOrient();
  void SaveToOrient();


protected:
  DECLARE_EVENT_TABLE();
private:
  SpinXML::Orientation mOrient;
  bool mLoading;
};

//============================================================//
// OrientEditDialog

class OrientEditDialog : public OrientDialogBase {
public:
  OrientEditDialog(wxWindow* parent,wxWindowID id=-1);

  int ShowModal();

  void SetOrient(const SpinXML::Orientation& orient) {mPanel->SetOrient(orient);}
  const SpinXML::Orientation& GetOrient() {return mPanel->GetOrient();}

  void OnApply(wxCommandEvent& e);
protected:
  DECLARE_EVENT_TABLE();
private:
  OrientEditPanel* mPanel;
};

//============================================================//
// OrientDialogCombo

class OrientDialogCombo : public DialogCombo<OrientEditDialog> {
public:
  OrientDialogCombo(wxWindow* parent,wxWindowID id=-1);

  void SetOrient(const SpinXML::Orientation& orient);
  const SpinXML::Orientation& GetOrient() {return mOrient;}

  virtual void SetStringValue(const wxString& s);

protected:
  virtual OrientEditDialog* CreateDialog();
  virtual wxString GetStringFromDialog(OrientEditDialog* dlg);
private:
  SpinXML::Orientation mOrient;
};

#endif
