
#include <gui/SpinachApp.hpp>
#include <gui/SpinGrid.hpp>
#include <shared/nuclear_data.hpp>
#include <iostream>
#include <gui/RightClickMenu.hpp>
#include <wx/debug.h>

using namespace std;

using namespace SpinXML;

const long ColumCount=10;  

//============================================================//
// Utility functions.

//Enum for use with wxString FormatLinearInteractions()
enum AlgebrakeForm {
  LINEAR,
  BILINEAR,
  QUAD,
  ALL
};

//Write a string representing the interactions acting on a spin
wxString FormatInteractions(Spin* spin,AlgebrakeForm form=ALL) {
  wxString str;
  vector<Interaction*> Interactions=spin->GetInteractions();
  long count=Interactions.size();
  bool first=true;
  for(long i=0;i<count;i++) {

    Interaction* inter=Interactions[i];
    AlgebrakeForm f=(inter->GetIsLinear() ?
		     LINEAR : (inter->GetIsBilinear() ? BILINEAR :
			       (QUAD)));
    if(f==form || form==ALL) {
      if(!first) {
	str << wxT(", ");
      }
      first=false;
      str << wxString(Interaction::GetSubTypeName(inter->GetSubType()),wxConvUTF8);
    }
  }
  return str;
}


class SpinGridRow : public sigc::trackable {
public:
  SpinGridRow(SpinGrid* parent,Spin* spin,long row) 
    : mParent(parent),
      mSpin(spin),
      rowNumber(row) {
    parent->sigDying.connect(mem_fun(this,&SpinGridRow::OnGridDying));
    parent->sigClearing.connect(mem_fun(this,&SpinGridRow::OnGridDying));

    spin->sigChange.connect(mem_fun(this,&SpinGridRow::UpdateRow));
    spin->sigDying.connect(mem_fun(this,&SpinGridRow::OnSpinDying));
    UpdateRow();
  }

  void ChangeRow(long newRow) {
    if(rowNumber<0 || rowNumber>=GetSS()->GetSpinCount()) {
      return;
    }
    rowNumber=newRow;
    UpdateRow();
  }

  void OtherRowsDeleted(int pos,int number) {
    if(pos<rowNumber) {
      rowNumber-=number;
    }
  }
  void UpdateRow() {
    double x,y,z;
    mSpin->GetCoordinates(&x,&y,&z);

    //Setup the label and the element columns
    mParent->SetCellValue(rowNumber,SpinGrid::COL_LABEL,wxString(mSpin->GetLabel(),wxConvUTF8));

    //Setup the x,y,z coordinates
    mParent->SetCellValue(rowNumber,SpinGrid::COL_X,wxString() << x);
    mParent->SetCellValue(rowNumber,SpinGrid::COL_Y,wxString() << y);
    mParent->SetCellValue(rowNumber,SpinGrid::COL_Z,wxString() << z);

    //Set the element and isotope
    long element=mSpin->GetElement();
    wxString str(getElementSymbol(element),wxConvUTF8);
    str << wxT(" ") << wxString(getElementName(element),wxConvUTF8);
    mParent->SetCellValue(rowNumber,SpinGrid::COL_ELEMENT,str);

    //Set the interactions
    mParent->SetCellValue(rowNumber,SpinGrid::COL_LINEAR     ,FormatInteractions(mSpin,LINEAR));
    mParent->SetCellValue(rowNumber,SpinGrid::COL_BILINEAR   ,FormatInteractions(mSpin,BILINEAR));
    mParent->SetCellValue(rowNumber,SpinGrid::COL_QUAD,       FormatInteractions(mSpin,QUAD));
  }
  
  void OnSpinChange() {
    UpdateRow();
  }
  void OnSpinDying(Spin* /*unused*/) {
    cout << "A gird row knows a spin" <<  rowNumber << " just died" << endl;
    if(mParent->DeleteRows(rowNumber,1)) {
      cerr << "Error deleting grid row" << endl;
    }
    delete this;
  }
  void OnGridDying() {
    delete this;
  }

private:
  long rowNumber;
  Spin* mSpin;
  SpinGrid* mParent;
};


//============================================================//
// SpinGrid

const SpinGrid::SpinGridColum SpinGrid::columns[]={
  {COL_SELECTED,   "Selected",73},
  {COL_LABEL,      "Label",105},    
  {COL_ELEMENT,    "Element",70},   
  {COL_ISOTOPES,   "Isotopes",70},  
  {COL_X,          "x",70},	    
  {COL_Y,          "y",70},	    
  {COL_Z,          "z",70},	    
  {COL_LINEAR,     "Linear",70},    
  {COL_BILINEAR,   "Bilinear",70},
  {COL_QUAD,"Quadrapolar",100}
};		   



SpinGrid::SpinGrid(wxWindow* parent,wxWindowID id)
  :wxGrid(parent,id),mUpdating(false),mSS(GetSS()) {

  CreateGrid(0, ColumCount);

  EnableEditing(true);
  EnableGridLines(true);
  EnableDragGridSize(true);
  EnableDragColMove(false);
  EnableDragColSize(true);

  SetMargins(0,0); 
  SetColLabelSize(20);

  for(long i=0;i<ColumCount;i++) {
    SetColSize(i,columns[i].width);
    SetColLabelValue(i,wxString(columns[i].name,wxConvUTF8));
  }

  SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
  	
  SetRowLabelSize(40);
  SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );

  mSS->sigReloaded.connect(mem_fun(this,&SpinGrid::RefreshFromSpinSystem));
  RefreshFromSpinSystem();
}
    

void SpinGrid::OnEdit(wxGridEvent& e) {
  long sc=GetSS()->GetSpinCount();
  if(e.GetRow()==sc) {
    //User is trying to edit the blank line at the bottom of the grid,
    //so create a new spin for them
    GetSS()->InsertSpin(new Spin(Vector3(0,0,0),"New Spin",1));
  }
}

void SpinGrid::OnNewSpin(Spin* newSpin,long number) {
  //Somehow, somewhere a new spin has been created, so create a new
  //row for it at the end of the grid
  long sc=GetSS()->GetSpinCount();
  SetupRow(sc);
  AppendRows(1);
  sigRowDelete.connect(mem_fun(
			       new SpinGridRow(this,GetSS()->GetSpin(sc-1),sc-1),
			       &SpinGridRow::OtherRowsDeleted
			       ));

}

void SpinGrid::OnEndEdit(wxGridEvent& e) {
  if(e.GetCol()==COL_LINEAR or e.GetCol()==COL_QUAD) {
    cout << "OnEndEdit" << endl;
  }
}

bool SpinGrid::DeleteRows(int pos,int numRows,bool updateLables) {
  bool r=wxGrid::DeleteRows(pos,numRows,updateLables);
  sigRowDelete(pos,numRows);
  return r;
}

void SpinGrid::RefreshFromSpinSystem() {
  mUpdating=true;

  GetSS()->sigNewSpin.connect(mem_fun(this,&SpinGrid::OnNewSpin));

  if(GetNumberRows()) {
    //Clear grid only clears the underlying data rather. The cells
    //remain and are black. wxGrid::DeleteRows solves this
    DeleteRows(0,GetNumberRows());
  }
  long count=GetSS()->GetSpinCount()+1;
  AppendRows(count);
  for (long i=0; i < count; i++) {
    SetupRow(i);
    if(i<count-1) {
	sigRowDelete.connect(mem_fun(
				     new SpinGridRow(this,GetSS()->GetSpin(i),i),
				     &SpinGridRow::OtherRowsDeleted
				     ));
	
    }
  }
  mUpdating=false;
}

void SpinGrid::SetupRow(long rowNumber) {
  //Set the selected column renderers and editors to be boolian
  SetCellRenderer(rowNumber,COL_SELECTED,new wxGridCellBoolRenderer());
  SetCellEditor(rowNumber,COL_SELECTED,new wxGridCellBoolEditor());

  //
  long count=getElementCount();

  wxArrayString strArray;
  for(long i=0;i<count;i++) {
    wxString str(getElementSymbol(i),wxConvUTF8);
    str << wxT(" ") << wxString(getElementName(i),wxConvUTF8);
    strArray.Add(str);
  }

  SetCellEditor(rowNumber,COL_ELEMENT,new wxGridCellChoiceEditor(strArray));

  //Set up floating point editors, so the user can't enter something stupid
  SetCellEditor(rowNumber,COL_X,new wxGridCellFloatEditor());
  SetCellEditor(rowNumber,COL_Y,new wxGridCellFloatEditor());
  SetCellEditor(rowNumber,COL_Z,new wxGridCellFloatEditor());

  //The Linear, Bilienar and Quad colums should not be editable
  SetReadOnly(rowNumber,COL_LINEAR);
  SetReadOnly(rowNumber,COL_BILINEAR);
  SetReadOnly(rowNumber,COL_QUAD);
}

void SpinGrid::OnCellChange(wxGridEvent& e) {
  if(mUpdating) {
    return;
  }
  if(e.GetCol()==COL_X) {
    double x;
    GetCellValue(e.GetRow(),e.GetCol()).ToDouble(&x);
    Chkpoint(wxT("Spin Coordinates"));
    mSS->GetSpin(e.GetRow())->GetPosition().SetX(x);
  } else if(e.GetCol()==COL_Y) {
    double y;
    GetCellValue(e.GetRow(),e.GetCol()).ToDouble(&y);
    Chkpoint(wxT("Spin Coordinates"));
    mSS->GetSpin(e.GetRow())->GetPosition().SetY(y);
  } else if(e.GetCol()==COL_Z) {
    double z;
    GetCellValue(e.GetRow(),e.GetCol()).ToDouble(&z);
    Chkpoint(wxT("Spin Coordinates"));
    mSS->GetSpin(e.GetRow())->GetPosition().SetZ(z);
  } else if(e.GetCol()==COL_ELEMENT) {
    wxString content=GetCellValue(e.GetRow(),e.GetCol());
    long space=content.Find(wxT(" "));
    wxString symbol=content.SubString(0,space-1);
    long element=getElementBySymbol(symbol.char_str());
    if(element==-1) {
      cerr << "Reverse lookup of element symbol " << symbol.char_str() << " failed!" << endl;
    } else {
      UpdateRowIsotopes(e.GetRow());
      Chkpoint(wxT("Spin Element"));
      mSS->GetSpin(e.GetRow())->SetElement(element);
    }
    cout << space << " " << symbol.char_str() << endl;
  } else if(e.GetCol()==COL_LABEL) {
    Chkpoint(wxT("Change Spin Label"));
    std::string label(GetCellValue(e.GetRow(),e.GetCol()).char_str());
    GetSS()->GetSpin(e.GetRow())->SetLabel(label);
  }
}

void SpinGrid::UpdateRowIsotopes(long row) {

}

void SpinGrid::OnCellSelect(wxGridEvent& e) {
  if(e.GetRow()==GetSS()->GetSpinCount()) {
    //The user clicked the last line, which means there is no spin to
    //edit
    e.Skip();
    return;
  }
  sigSelect(columns[e.GetCol()].type,GetSS()->GetSpin(e.GetRow()));
  e.Skip();
}

void SpinGrid::OnRightClick(wxGridEvent& e) {
  RightClickMenu* menu=new RightClickMenu(this);
  if(e.GetRow()<mSS->GetSpinCount()) {
    menu->OptionDeleteSpin        (GetSS()->GetSpin(e.GetRow()));
    menu->OptionShowSpinProperties(GetSS()->GetSpin(e.GetRow()));
  }
  menu->Build();
  PopupMenu(menu);
  delete menu;
}

void SpinGrid::OnDeleteSpinHover(wxCommandEvent& e) {
}


BEGIN_EVENT_TABLE(SpinGrid,wxGrid)

EVT_GRID_SELECT_CELL     (         SpinGrid::OnCellSelect)
EVT_GRID_CMD_CELL_CHANGE (wxID_ANY,SpinGrid::OnCellChange)
EVT_GRID_EDITOR_HIDDEN   (         SpinGrid::OnEndEdit)
EVT_GRID_EDITOR_SHOWN    (         SpinGrid::OnEdit)

EVT_GRID_CELL_RIGHT_CLICK(         SpinGrid::OnRightClick)

END_EVENT_TABLE()


//============================================================//
// SpinGridPanel

SpinGridPanel::SpinGridPanel(wxWindow* parent,wxWindowID id) 
: wxPanel(parent,id){
  wxBoxSizer* sizer=new wxBoxSizer(wxVERTICAL);

  //NB: Construction must be in this order as wxGrid will emit a cell
  //selection event as they are being constucted. This would cause
  //this->OnInterUnSelect to be called which tries to do something
  //with mInterEdit which is currently uninialised.
  //
  //The bright side is that we don't need to explicity call
  //mInterEdit->SetSpin as the correct behavior obtained by default
  //(so long as mInterEdit is constructed)
  mInterEdit=new SpinInterEditPanel(this);
  mInterEdit->SetDialogMode(false);
  mGrid=new SpinGrid(this);

  sizer->Add(mGrid,1,wxEXPAND | wxALL);
  sizer->Add(mInterEdit,0,wxEXPAND | wxALL);

  SetSizer(sizer);

  mGrid->sigSelect.connect(mem_fun(*this,&SpinGridPanel::OnGridCellSelect));
}

void SpinGridPanel::OnGridCellSelect(SpinGrid::COL_TYPE col,Spin* spin) {
  mInterEdit->Show(col==SpinGrid::COL_LINEAR   ||
		   col==SpinGrid::COL_BILINEAR ||
		   col==SpinGrid::COL_QUAD);
  mInterEdit->SetSpin(spin);
  Layout();
}

