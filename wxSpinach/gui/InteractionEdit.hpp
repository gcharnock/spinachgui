
#ifndef INTEREDIT_H
#define INTEREDIT_H

#include <gui/OrientationEdit.hpp>

#include <res/SpinachGUI.h>
#include <shared/spinsys.hpp>

#include <gui/DialogCombo.hpp>


class InterEditPanel : public InterEditPanelBase {
public:
  InterEditPanel(wxWindow* parent, wxWindowID id=-1);

  void SetInter(SpinXML::Interaction* inter);

  void OnQuadChecked(wxCommandEvent& e);
  void OnPageChange(wxChoicebookEvent& e);
  void LoadFromInter();
  void SaveToInter();
  void onTextChange(wxCommandEvent& e);

protected:
  DECLARE_EVENT_TABLE();
private:
  //GUI Stuff
  OrientDialogCombo* mOrientEigenvalueCtrl;
  OrientDialogCombo* mOrientAxRhomCtrl;
  OrientDialogCombo* mOrientSpanSkewCtrl;

  SpinXML::Interaction* mInter;

  void UpdateSubTypeCombo();

  bool mLoading;
};



class SpinInterEditPanel : public SpinInterEditPanelBase {
public:
  SpinInterEditPanel(wxWindow* parent,SpinXML::Spin* spin,wxWindowID id= -1);
  ~SpinInterEditPanel();

  void OnNewButton(wxCommandEvent& e);
  void OnDeleteButton(wxCommandEvent& e);
  void OnInteractionChange(wxCommandEvent& e);
  void OnSSChange(wxCommandEvent& e);

  void MarkDeleted(long n);

  void CreateNew();

  void LoadFromSpin();
  void SaveToSpin();

  void UpdateListBox();
  void InteractionChange();

  enum EDIT_MODE {
    EDIT_ALL,
    EDIT_LINEAR,
    EDIT_BILINEAR,
    EDIT_QUAD
  };
  void SetEditMode(EDIT_MODE mEditMode);


protected:
  void Clear();

  DECLARE_EVENT_TABLE();

private:
  //GUI Stuff
  InterEditPanel* mInterEdit;
  ///Are we editing linear, bilinear, quadrapolar or all the
  ///interactions which apply to this spin
  EDIT_MODE mEditMode;
  ///A mapping of the list box index to the interaction index.
  ///Updated when UpdateListBox() is called. Will be the identity
  ///mapping if the edit mode is EDIT_ALL
  std::vector<long> mLBIndex2SpinIndex;

  long GetSelectedInterIndex() const;

  //Data

  struct ListBoxInteraction {
    SpinXML::Interaction* inter;
    bool modified;
  };


  SpinXML::Spin* mSpin;
  
  ///True when the GUI list box is being updated.  Events generated
  ///when mUpdatingListBox is true should usually be ignored
  bool mUpdatingListBox;
  std::vector<ListBoxInteraction> mTempInteractions;
};

class InterPopup : public wxFrame {
public:
  InterPopup(wxWindow* Parent,SpinXML::Spin* spin,wxWindowID id=-1);
private:
  SpinInterEditPanel* mPanel;
};


#endif
