


#3rd Party/standard Imports
import wx
from wx import xrc 
import wx.grid

import spinsys

#Local python imports
from utils import *

INTERTYPE_EULER=0;
INTERTYPE_ANGLEAXIS=1;
INTERTYPE_QUATERNION=2;
INTERTYPE_EIGENSYSTEM=3;

class EulerEdit(wx.Panel):
    def __init__(self,parent,id=-1):
        pre = wx.PrePanel();
        wx.GetApp().res.LoadOnPanel(pre,parent,"EulerPanel");
        self.PostCreate(pre);


class AngleAxisEdit(wx.Panel):
    def __init__(self,parent,id=-1):
        pre = wx.PrePanel();
        wx.GetApp().res.LoadOnPanel(pre,parent,"AngleAxisPanel");
        self.PostCreate(pre);


class QuaternionEdit(wx.Panel):
    def __init__(self,parent,id=-1):
        pre = wx.PrePanel();
        wx.GetApp().res.LoadOnPanel(pre,parent,"QuaternionPanel");
        self.PostCreate(pre);


class EigensystemEdit(wx.Panel):
    def __init__(self,parent,id=-1):
        pre = wx.PrePanel();
        wx.GetApp().res.LoadOnPanel(pre,parent,"EigensystemPanel");
        self.PostCreate(pre);



class OrientTransientPopup(wx.PopupTransientWindow):
    """Adds a bit of text and mouse movement to the wx.PopupWindow"""
    def __init__(self, parent, style):
        wx.PopupTransientWindow.__init__(self, parent, style)

        self.sizer=wx.BoxSizer()
        self.orientPanel=OrientEdit(self)
        self.sizer.Add(self.orientPanel,1.0,wx.EXPAND);
        self.SetSizer(self.sizer)
        

        size = self.orientPanel.GetSize()
        print size
        self.SetSize((size.width, size.height));

    def ProcessLeftDown(self, e):
        e.Skip()
        return False

    def OnDismiss(self):
        pass


class OrientTextEditor(wx.TextCtrl):
    def __init__(self,parent,id=-1):
        wx.TextCtrl.__init__(self,parent,id)
        self.Bind(wx.EVT_SET_FOCUS, self.OnShowPopupTransient, self)


    def OnShowPopupTransient(self, e):
        win = OrientTransientPopup(self,wx.SIMPLE_BORDER)

        # Show the popup right below or above the button
        # depending on available screen space...
        pos = self.ClientToScreen((0,0))
        size =  self.GetSize()
        win.Position(pos, (0, size[1]))

        win.Popup()


class OrientEdit(wx.Panel):
    def __init__(self,parent,id=-1):

        pre = wx.PrePanel();
        wx.GetApp().res.LoadOnPanel(pre,parent,"OrientEditPanel");
        self.PostCreate(pre)

        self.typeChoiceBook=xrc.XRCCTRL(self,"OrientChoiceBook");

        self.eulerPage=      wx.GetApp().res.LoadPanel(self.typeChoiceBook,"EulerPanel")
        self.angleAxisPage=  wx.GetApp().res.LoadPanel(self.typeChoiceBook,"AngleAxisPanel")
        self.quaternionPage= wx.GetApp().res.LoadPanel(self.typeChoiceBook,"QuaternionPanel")
        self.eigensystemPage=wx.GetApp().res.LoadPanel(self.typeChoiceBook,"EigensystemPanel")

        self.typeChoiceBook.AddPage(self.eulerPage,"Euler")
        self.typeChoiceBook.AddPage(self.angleAxisPage,"Angle Axis")
        self.typeChoiceBook.AddPage(self.quaternionPage,"Quaternion")
        self.typeChoiceBook.AddPage(self.eigensystemPage,"Eigensystem")

        
        #Build the five "plug in" panels for the five different types of interaction
        

