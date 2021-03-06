
#include <gui/RightClickMenu.hpp>
#include <gui/SpinachApp.hpp>

using namespace SpinXML;
using namespace std;

//============================================================//
// RightClickAction

//-- Lambda

RCActionLambda::RCActionLambda(wxString str,CommandT command)
    : RightClickAction(str),mCommand(command) {
}

void RCActionLambda::Exec(wxCommandEvent& e) {
    mCommand(e);
}

//-- Delete Selection

void RCActionDeleteSelection::Exec(wxCommandEvent&e) {
    DeleteSelectedSpins();
}

bool RCActionDeleteSelection::Visible() const {
    return GetSelectedCount() >=0;
}


//-- Delete Hover

RCActionDeleteHover::RCActionDeleteHover()
    : RightClickAction(wxT("Delete Spin (under mouse)")),
      mHoverAtTimeOfOpening(GetHover()) {
}

void RCActionDeleteHover::Exec(wxCommandEvent& e) {
    delete mHoverAtTimeOfOpening;
}


//============================================================//
// PopupMenu


RightClickMenu::RightClickMenu(wxWindow* parent)
    : wxMenu(),mParent(parent) {
    Connect(wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(RightClickMenu::OnSelect));
}

 RightClickMenu::~RightClickMenu() {
    for(std::vector<RightClickAction*>::iterator i = mActions.begin();i!=mActions.end();++i) {
        delete *i;
    }
 }

void RightClickMenu::Build(std::vector<RightClickAction*> actions) {
    mActions = actions;
    int id = 0;
    for(std::vector<RightClickAction*>::iterator i = actions.begin();i!=actions.end();++i) {
        if ((*i)->Visible()) {
            Append(id, (*i)->mText);
        }
        id++;
    }
}

void RightClickMenu::OnSelect(wxCommandEvent& e) {
    mActions[e.GetId()]->Exec(e);
}


void RightClickMenu::Build() {
    vector<RightClickAction*> actions;
    actions.push_back(new RCActionDeleteHover                );
    actions.push_back(new RCActionDeleteSelection            );
    Build(actions);
}


