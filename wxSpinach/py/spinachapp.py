import wx
import wx.aui
from wx import xrc 
from wx import glcanvas 
import wx.grid

import spinsys
from nuclear_data import *
from math import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *

from numpy import *

import time


#Define some openGL materials
whiteMaterial = array([0.5, 0.5, 0.5],float32); 
blueMaterial = array([0.06, 0.06, 0.4],float32); 
redMaterial = array([0.9, 0.00, 0.00],float32); 

hoverMaterial=redMaterial
selectedMaterial=blueMaterial

def splitSymbol(symbol):
    """Split and isotope symbol such as H1 into a letter and a number such as (H,1)"""
    str="";
    num="";
    for c in symbol:
        if c.isalpha():
            str=str+c
        else:
            num=num+c
    return (str,num)



def getARotation(parent):
    class rotDialog():
        def __init__(self):
            self.orientRes = xrc.XmlResource("res/gui.xrc")
            self.dlg = self.orientRes.LoadDialog(parent, "rotationDialog")


            self.eulerAPanel = xrc.XRCCTRL(self.dlg,'eulerAPanel')
            self.eulerBPanel = xrc.XRCCTRL(self.dlg,'eulerBPanel')
            self.angleAxisPanel = xrc.XRCCTRL(self.dlg,'angleAxisPanel')

            self.radioBox = xrc.XRCCTRL(self.dlg,'mRConRadioBox')

            self.eulerBPanel.Enable(False)
            self.angleAxisPanel.Enable(False)

            self.dlg.Bind(wx.EVT_RADIOBOX,self.onRadioBox,id=xrc.XRCID('mRConRadioBox'))

            
        def onRadioBox(self,e):
            self.eulerAPanel.Enable(False)
            self.eulerBPanel.Enable(False)
            self.angleAxisPanel.Enable(False)
            if e.Selection==0:
                self.eulerAPanel.Enable(True)
            elif e.Selection==1:
                self.eulerBPanel.Enable(True)
            elif e.Selection==2:
                self.angleAxisPanel.Enable(True)
            else:
                raise "A highly usual selection number was returned by a radioBox"
        def getRotation(self):
            convention=thisDialog.radioBox.GetSelection()
            if convention==0:
                v1=xrc.XRCCTRL(self.dlg,'alphaA')
                v2=xrc.XRCCTRL(self.dlg,'alphaA')
                v3=xrc.XRCCTRL(self.dlg,'gammaA')
            elif convention==1:
                v1=xrc.XRCCTRL(self.dlg,'alphaB')
                v2=xrc.XRCCTRL(self.dlg,'alphaB')
                v3=xrc.XRCCTRL(self.dlg,'gammaB')
            elif convention==2:
                v1=xrc.XRCCTRL(self.dlg,'omega')
                v2=xrc.XRCCTRL(self.dlg,'theta')
                v3=xrc.XRCCTRL(self.dlg,'phi')
            else:
                raise "A highly usual selection number was returned by a radioBox"
            return (convention,(float(v1.GetValue()),float(v2.GetValue()),float(v3.GetValue())))

    thisDialog=rotDialog()
    if(thisDialog.dlg.ShowModal() == wx.ID_OK):
        print "Dialog Okay"
        return thisDialog.getRotation()
    else:
        print "Dialog canceled"
        return None


class SpinGridEditPanel(wx.Panel):
    def __init__(self,parent,ss,res,id=-1):
        wx.Panel.__init__(self,parent,id)
        self.res=res
        self.ss=ss

	self.Sizer=wx.BoxSizer( wx.VERTICAL );

        self.grid=wx.grid.Grid(self,-1)

        self.grid.CreateGrid( self.ss.getSpinCount(), 7 );

        self.grid.EnableEditing( False );
        self.grid.EnableGridLines( True );
        self.grid.EnableDragGridSize( True );

        self.grid.SetMargins( 0, 0 ); 

        self.grid.SetColSize( 0, 73 );
        self.grid.SetColSize( 1, 105 );
        self.grid.SetColSize( 2, 70 );
        self.grid.SetColSize( 3, 75 );
        self.grid.SetColSize( 4, 50 );
        self.grid.SetColSize( 5, 50 );
        self.grid.SetColSize( 6, 50 );

        self.grid.EnableDragColMove(False);
        self.grid.EnableDragColSize(True);
        self.grid.SetColLabelSize( 30 );

        self.grid.SetColLabelValue( 0, "Selected" );
        self.grid.SetColLabelValue( 1, "Spin Number" );
        self.grid.SetColLabelValue( 2, "Element" );
        self.grid.SetColLabelValue( 3, "Isotopes" );
        self.grid.SetColLabelValue( 4, "x" );
        self.grid.SetColLabelValue( 5, "y" );
        self.grid.SetColLabelValue( 6, "z" );
        self.grid.SetColLabelAlignment( wx.ALIGN_CENTRE, wx.ALIGN_CENTRE );
	
        self.grid.SetRowLabelSize( 80 );
        self.grid.SetRowLabelAlignment( wx.ALIGN_CENTRE, wx.ALIGN_CENTRE );
	
        self.SetSizer(self.Sizer);
        self.Sizer.Add(self.grid,0,wx.ALL | wx.EXPAND,0)


class glDisplay(wx.glcanvas.GLCanvas):
    def __init__(self,parent,id=-1):
        super(glDisplay,self).__init__(parent,id)
        self.parent=parent
        self.ss=None
        self.xRotate=0
        self.yRotate=0
        self.rotationMatrix=array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0, 0, 0, 1]], float64)
        self.zoom=0.01
        self.camX=0.0
        self.camY=0.0
        self.camZ=15.0
        self.mousex=0;
        self.mousey=0;
        self.xTranslate=0;
        self.yTranslate=0;
        self.selected=[]  #List containing all spins which are currently selected
        self.hover=-1   #The closest spin currently sitting under the mouse

    def setSpinSys(self,ss):
        """Set the spin system that this gl display is displaying"""
        self.ss=ss

    def setDrawMode(self,mode):
        if mode=="wireframe":
            glDisable(GL_DEPTH_TEST)  
            glDisable(GL_LIGHTING);   
            glDisable(GL_LIGHT0);     
            glDisable(GL_LIGHT1);     
        else:
            glEnable(GL_DEPTH_TEST)  
            glEnable(GL_LIGHTING);   
            glEnable(GL_LIGHT0);     
            glEnable(GL_LIGHT1);     
        self.mode=mode

    def enableGL(self):
        """This has to be called after the frame has been shown"""
        self.SetCurrent()
	glClearColor(0.0, 0.0, 0.0, 0.0);
        self.setDrawMode('self')

        self.sphereWire = glGenLists(1);
       
        qobj = gluNewQuadric();

        gluQuadricDrawStyle(qobj,GLU_LINE);
        gluQuadricNormals(qobj,GLU_SMOOTH);

        glNewList(self.sphereWire,GL_COMPILE);
        gluSphere(qobj,1.0,14,14);
        glEndList();
        gluDeleteQuadric(qobj);

        self.sphereSolid = glGenLists(1);

        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj,GLU_FILL);
        gluQuadricNormals(qobj,GLU_SMOOTH);

        glNewList(self.sphereSolid,GL_COMPILE);
        gluSphere(qobj,1.0,12,12);
        glEndList();
        gluDeleteQuadric(qobj);



	glDepthFunc(GL_LEQUAL);

        #Reserve Space for the display lists
        self.bondDispList=glGenLists(1);

	#glShadeModel(GL_SMOOTH);
        #Adpated from a detailed tutorail on opengl lighting located at
        #http://www.falloutsoftware.com/tutorials/gl/gl8.htm


	# We're setting up two light sources. One of them is located
	# on the left side of the model (x = -1.5f) and emits white light. The
	# second light source is located on the right side of the model (x = 1.5f)
	# emitting red light.

	# GL_LIGHT0: the white light emitting light source
	# Create light components for GL_LIGHT0
	ambientLight0 =  array([0.0, 0.0, 0.0, 1.0],float32);
	diffuseLight0 =  array([0.5, 0.5, 0.5, 1.0],float32);
	specularLight0 = array([0.6, 0.6, 0.6, 1.0],float32);
	position0 =      array([-1.5, 1.0,-4.0, 1.0],float32);	
	# Assign created components to GL_LIGHT0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight0);
	glLightfv(GL_LIGHT0, GL_POSITION, position0);

	# GL_LIGHT1: the red light emitting light source
	# Create light components for GL_LIGHT1
	ambientLight1 =  array([0.1, 0.1, 0.1, 1.0 ],float32);
	diffuseLight1 =  array([0.1, 0.1, 0.1, 1.0 ],float32);
	specularLight1 = array([0.3, 0.3, 0.3, 1.0 ],float32);
	position1 =      array([1.5, 1.0, 4.0,1.0],float32);	
	# Assign created components to GL_LIGHT1
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight1);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);

        #glEnable(GL_COLOR_MATERIAL);
        #glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

        self.Bind(wx.EVT_PAINT,self.onPaint)
        self.Bind(wx.EVT_MOTION,self.onMouseMove)
        self.Bind(wx.EVT_MOUSEWHEEL,self.onWheel)
        self.Bind(wx.EVT_RIGHT_UP,self.onRightClick)
        self.Bind(wx.EVT_LEFT_UP,self.onLeftClick)

        #Create a dictionary of colours
        self.colourDict={}
        for i in range(getElementCount()):
            self.colourDict[getElementSymbol(i)]=array([getElementR(i), getElementG(i), getElementB(i)],float32);

        self.genBondList()

    def onWheel(self,e):
        self.zoom=self.zoom-0.001*e.GetWheelRotation()/e.GetWheelDelta();
        if(self.zoom<0.001):
            self.zoom=0.001;
        self.Refresh()

    def genBondList(self):
        self.bondDispList
        glNewList(self.bondDispList,GL_COMPILE);

        qobj = gluNewQuadric();
        if self.mode=="wireframe":
            gluQuadricDrawStyle(qobj,GLU_LINE);
        else:
            gluQuadricDrawStyle(qobj,GLU_FILL);
        gluQuadricNormals(qobj,GLU_SMOOTH);

        for i in range(self.ss.getSpinCount()):   #Draw the spins and the bonds
            thisSpin=self.ss.getSpinByIndex(i)
            coords=thisSpin.getCoords()
        
            #draw in bonds to nearby atoms

            glMaterialfv(GL_FRONT, GL_SPECULAR, blueMaterial);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, whiteMaterial);
            nearby=self.ss.getNearbySpins(i,1.8)
            glColor3f(1.0, 0.0, 0.0);
            for index in nearby:
                otherCoords=self.ss.getSpinByIndex(index).getCoords()
                bondLength=((coords[0]-otherCoords[0])*(coords[0]-otherCoords[0])+
                            (coords[1]-otherCoords[1])*(coords[1]-otherCoords[1])+
                            (coords[2]-otherCoords[2])*(coords[2]-otherCoords[2]))**0.5

               #Now we need to find the rotation between the z axis
                angle=acos((otherCoords[2]-coords[2])/bondLength)
                glPushMatrix();
                glTranslatef(coords[0],coords[1],coords[2]);
                glRotate(angle/2/pi*360,coords[1]-otherCoords[1],otherCoords[0]-coords[0],0)
                gluCylinder(qobj,0.1,0.1,bondLength,7,7)
                glPopMatrix();

        glEndList();


    def onPaint(self,e):
        t1=time.time()
	glColor3f(0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT)
        glClear(GL_DEPTH_BUFFER_BIT)
	glClearDepth(1.0);

        width,height = self.GetClientSizeTuple()
        glViewport(0,0,width,height);

        glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
        glOrtho(-width*self.zoom, width*self.zoom, -height*self.zoom, height*self.zoom, -40.0, 40.0);
        #gluPerspective(45.0,float(width)/float(height),0.1, 200.0);
        glMatrixMode(GL_MODELVIEW);


        #Take the opertunity to calculate the rotation matrix for the scene
        #TODO: This would be better handled on the CPU, it's only one
        #      matrix. Change when matrix classes have been written
        dotProduct=(self.xRotate*self.xRotate+self.yRotate*self.yRotate);
        norm=sqrt(dotProduct);

        glLoadIdentity();
        glRotatef(dotProduct,self.yRotate/norm,self.xRotate/norm,0);
        glTranslatef(self.xTranslate*self.zoom,self.yTranslate*self.zoom,0);
        self.xRotate=0
        self.yRotate=0
        self.xTranslate=0
        self.yTranslate=0
        glMultMatrixf(self.rotationMatrix);
        self.rotationMatrix=glGetFloatv(GL_MODELVIEW_MATRIX);

        glLoadIdentity();
        gluLookAt(self.camX,self.camY,self.camZ,0,0,-1,0,1,0);
        glMultMatrixf(self.rotationMatrix);

        if self.ss==None:
            self.SwapBuffers()
            return

        spinCount=self.ss.getSpinCount()

        qobj = gluNewQuadric();
        if self.mode=="wireframe":
            gluQuadricDrawStyle(qobj,GLU_LINE);
        else:
            gluQuadricDrawStyle(qobj,GLU_FILL);
        gluQuadricNormals(qobj,GLU_SMOOTH);

        #Move these to some sort of config file
        radius=0.3
        radius2=radius*radius


        #Find out if any of the spins are under the mouse and record the closest
        self.hover=-1     #The index of the spin the user is hovering over
        self.hoverDist=0  #This distance to that spin

        viewport   = glGetIntegerv(GL_VIEWPORT);
        mvmatrix   = glGetDoublev(GL_MODELVIEW_MATRIX);
        projmatrix = glGetDoublev(GL_PROJECTION_MATRIX);
        worldFarX, worldFarY ,worldFarZ  = gluUnProject(self.mousex,height-self.mousey-1,1.0,mvmatrix,projmatrix,viewport);
        worldNearX,worldNearY,worldNearZ = gluUnProject(self.mousex,height-self.mousey-1,0.0,mvmatrix,projmatrix,viewport);
        tsetup=time.time()
        for i in range(spinCount):  #Decide which spin is selected
            thisSpin=self.ss.getSpinByIndex(i);
            coords=thisSpin.getCoords()

            #The distance from the near clipping plane is reused in the colision detection
            clipDist2=(worldNearX-coords[0])**2 +    (worldNearY-coords[1])**2 + (worldNearZ-coords[2])**2
            if clipDist2 > self.hoverDist and self.hover!=-1:
                continue; #We already found a closer spin, so there isn't any point in checking this one

            #Cast a ray from the eye to worldx,worldy,worldz and see if it collides with anything.
            #There four equations which combine into quadratic equation. If the descrimiate indicates a real
            #solution then the ray hits the sphere.
            Rx=worldNearX-worldFarX
            Ry=worldNearY-worldFarY
            Rz=worldNearZ-worldFarZ

            A =    Rx**2+                         Ry**2                    + Rz**2
            B = 2*(Rx*(worldNearX-coords[0]) +    Ry*(worldNearY-coords[1]) + Rz*(worldNearZ-coords[2]))
            C =    clipDist2 - radius2

            desc=B**2-4*A*C
            if desc > 0:
                self.hover=i
                self.hoverDist=clipDist2
            

        tspins=time.time()

        for i in range(spinCount):   #Draw the spins and the bonds
            thisSpin=self.ss.getSpinByIndex(i)
            coords=thisSpin.getCoords()

            glColor3f(1.0, 1.0, 1.0);
            #Draw in the single spin interaction tensor
            mat3=self.ss.GetTotalInteractionOnSpinAsMatrix(i)
            #Convert to a openGL 4x4 matrix
            mat=array([[abs(mat3.get(0,0)),abs(mat3.get(0,1)),abs(mat3.get(0,2)),0],
                       [abs(mat3.get(1,0)),abs(mat3.get(1,1)),abs(mat3.get(1,2)),0],
                       [abs(mat3.get(2,0)),abs(mat3.get(2,1)),abs(mat3.get(2,2)),0],
                       [0,0,0,1]],float32)
            #Apply the transformation matrix to warp the sphere
            #print mat
            glPushMatrix();

            glTranslatef(coords[0],coords[1],coords[2]);

            glPushMatrix();
            glMultMatrixf(mat)
            glScale(0.04,0.04,0.04)
            glCallList(self.sphereWire);
            glPopMatrix();

            eValX=self.ss.getEigenValX(i).real;
            eValY=self.ss.getEigenValY(i).real;
            eValZ=self.ss.getEigenValZ(i).real;

            eVecX=self.ss.getEigenVecX(i);
            eVecY=self.ss.getEigenVecY(i);
            eVecZ=self.ss.getEigenVecZ(i);

            #Draw the three eigenvectors of the interactionx
            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(eVecX[0]*eValX,eVecX[1]*eValX,eVecX[2]*eValX);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(eVecY[0]*eValY,eVecY[1]*eValY,eVecY[2]*eValY);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(eVecZ[0]*eValZ,eVecZ[1]*eValZ,eVecZ[2]*eValZ);
            glEnd();

            glPopMatrix();


            if(self.hover==i):
                glMaterialfv(GL_FRONT, GL_SPECULAR, redMaterial);
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, whiteMaterial);
            elif(i in self.selected):
                glMaterialfv(GL_FRONT, GL_SPECULAR, selectedMaterial);
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, whiteMaterial);                
            else:
                letter,num=splitSymbol(thisSpin.getIsotope())
                if (letter in self.colourDict):
                     glMaterialfv(GL_FRONT, GL_SPECULAR, self.colourDict[letter]);
                     glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, self.colourDict[letter]);
            glPushMatrix();
            glTranslatef(coords[0],coords[1],coords[2]);
            glScale(radius,radius,radius);
            if(self.mode=="wireframe"):
                glCallList(self.sphereWire);
            else:
                glCallList(self.sphereSolid);
            glPopMatrix();
        
        glCallList(self.bondDispList);

        tbonds=time.time()

        glDisable(GL_LIGHTING);   
        glDisable(GL_LIGHT0);     
        glDisable(GL_LIGHT1);     
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for i in range(spinCount):  #Draw the J couplings
            #Do the two spin couplings
            thisSpin=self.ss.getSpinByIndex(i)
            coords=thisSpin.getCoords()            
            for j in range(i+1,spinCount):
                coordsJ=self.ss.getSpinByIndex(j).getCoords()
                scalar=abs(self.ss.GetTotalIsotropicInteractionOnSpinPair(i,j))/300;
                glColor4f(scalar,0,0,scalar);
                glBegin(GL_LINES);
                glVertex3f(coords[0],coords[1],coords[2]);
                glVertex3f(coordsJ[0],coordsJ[1],coordsJ[2]);
                glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);   
        glEnable(GL_LIGHT0);     
        glEnable(GL_LIGHT1);     
                
                

        glColor3f(0.0, 0.0, 1.0);

        tj=time.time()

        self.SwapBuffers()
        t2=time.time()
        print ("Render time=" +str(t2-t1) + "ms , approx fps=" +str(1/(t2-t1)) + 
               "s^-1 (setup=" + str(tsetup-t1) + ", spins=" + str(tspins-tsetup) + ", bonds="+
               str(tbonds-tspins)+", j="+str(tj-tbonds)+", tswap="+str(t2-tj)+")")
        
    def onMouseMove(self,e):
        if(e.Dragging() and e.RightIsDown()):
            self.xTranslate=self.xTranslate+(e.GetX()-self.mousex)
            self.yTranslate=self.yTranslate-(e.GetY()-self.mousey)
        elif(e.Dragging() and e.LeftIsDown()):
            self.xRotate=self.xRotate+(e.GetX()-self.mousex);
            self.yRotate=self.yRotate+(e.GetY()-self.mousey);

        self.mousex=e.GetX();
        self.mousey=e.GetY();
        self.Refresh()

    def onLeftClick(self,e):
        if(not e.ShiftDown()):
            self.selected=[]
        if self.hover != -1:
            self.selected.append(self.hover);
        self.Refresh()


    def onRightClick(self,e):
        if(not e.Dragging()):
            menu = wx.Menu()
            menu.Append(-1, "Test Menu Item")
            if(self.hover>=0):
                menu.Append(-1, "Spin Properties...")
                menu.Bind(wx.EVT_MENU,self.parent.onDisplaySpinPropDialog);
            if(len(self.selected)==2):
                menu.Append(-1, "Coupling Properties")                    
   
            self.PopupMenuXY( menu, e.GetX(), e.GetY() )
            menu.Destroy()


class RootFrame(wx.Frame):
    def __init__(self,res,ssroot):
        self.res=res;
        self.ssroot=ssroot
        self.ss=self.ssroot.getRoot()

        pre = wx.PreFrame();
        self.res.LoadOnFrame(pre,None,"rootFrameBase");
        self.PostCreate(pre);
        self.SetSize(wx.Size(1024,768));
        self.init_frame();

    def init_frame(self):
        self.toolbar = xrc.XRCCTRL(self,'mRootToolbar')

        #Hide the uneeded panels
        self.spinPanel = xrc.XRCCTRL(self, 'mSpinPanel')
        self.JCoupPanel = xrc.XRCCTRL(self, 'mJCoupPanel')
        self.JCoupPanel.Show(False)
        self.ClusterPanel = xrc.XRCCTRL(self, 'mClusterPanel')
        self.ClusterPanel.Show(False)
        
        #Populate the spin tree
        self.spinTree = xrc.XRCCTRL(self,'mSpinTree')
        self.spinTree.AddRoot("Spin System")
        self.updateSpinTree()
        
        #Setup event handling
        self.Bind(wx.EVT_MENU, self.onSpinButton, id=xrc.XRCID('mSpinTool'))
        self.Bind(wx.EVT_MENU, self.onJButton, id=xrc.XRCID('mJTool'))
        self.Bind(wx.EVT_MENU, self.onClusterButton, id=xrc.XRCID('mClusterTool'))

        self.Bind(wx.EVT_MENU, self.onOpen, id=xrc.XRCID('mMenuItemOpen'))
        self.Bind(wx.EVT_MENU, self.onSave, id=xrc.XRCID('mMenuItemSave'))
        self.Bind(wx.EVT_MENU, self.onSaveAs, id=xrc.XRCID('mMenuItemSaveAs'))

        self.Bind(wx.EVT_MENU, self.onExit, id=xrc.XRCID('mMenuItemExit'))

        self.Bind(wx.EVT_MENU, self.onWireframe, id=xrc.XRCID('mMenuItemWireframe'))
        self.Bind(wx.EVT_MENU, self.onFilled, id=xrc.XRCID('mMenuItemFilled'))

        #Setup the aui elements (this cannot be done from wxFormBuilder currently)

        self.auiPanel=xrc.XRCCTRL(self,'auiPanel');
        self.notebook=wx.aui.AuiNotebook(self.auiPanel,-1);

        self.spinGrid=SpinGridEditPanel(self.notebook,self.ss,self.res)

        self.glc = glDisplay(self.notebook);
        self.dc=wx.PaintDC(self.glc);
        self.glc.setSpinSys(self.ss);


        # add the panes to the manager
        self.notebook.AddPage(self.glc, '3D View')
        self.notebook.AddPage(self.spinGrid, 'Grid View')

        self.auiPanel.GetSizer().Add(self.notebook,1,wx.EXPAND);

        #Set up a openGL canvas
        

        #Set up the grid

        #self.loadFromFile('data/hccch.xml')
        self.loadFromFile('data/tyrosine.log','g03')
        #self.loadFromFile('../../../testing_kit/Gaussian/NMR spectroscopy/molecule_9.log','g03');
        #self.saveToFile('data/tyrosine.xml')

        self.testDia=SpinPropDialog(self,self.ss,self.res,0);
        self.testDia.Show();

    def Show(self):
        wx.Frame.Show(self);
        self.glc.enableGL()   

 
    def updateSpinTree(self):
        count=self.ss.getSpinCount()
        print "Count=",count
        for i in range(count):
            spin=self.ss.getSpinByIndex(i)
            string=spin.getLabel() + " (" + spin.getIsotope()  + ")"
            self.spinTree.AppendItem(self.spinTree.GetRootItem(),string)

    def onSpinButton(self,e):
        self.spinPanel.Show(True)
        self.JCoupPanel.Show(False)
        self.ClusterPanel.Show(False)
        self.Layout();

    def onJButton(self,e):
        self.spinPanel.Show(False)
        self.JCoupPanel.Show(True)
        self.ClusterPanel.Show(False)
        self.Layout();

    def onClusterButton(self,e):
        self.spinPanel.Show(False)
        self.JCoupPanel.Show(False)
        self.ClusterPanel.Show(True)
        self.Layout();

    def onOpen(self,e):
        wildcard="Spin XML files (*.xml)|*.xml|G03 Log Files (*.log)|*.log|Plain XYZ Files (*.xyz)|*.xyz|All Files (*.*)|*.*"
        fd=wx.FileDialog(self,style=wx.FD_OPEN, message="Open a Spin System",wildcard=wildcard) 
        if(fd.ShowModal()):
            fileExt=fd.GetPath()[-3:]
            print fileExt
            if(fileExt=="log"):
                print "Gaussian"
                self.loadFromFile(fd.GetPath().encode('latin-1'),'g03')
            if(fileExt=="xyz"):
                self.loadFromFile(fd.GetPath().encode('latin-1'),'xyz')
            else:
                self.loadFromFile(fd.GetPath().encode('latin-1'))
            
    def loadFromFile(self,filename,type="xml"):
        if type=="xml":
            self.ssroot.loadFromFile(filename)
        elif type=="g03":
            self.ssroot.loadFromG03File(filename)
        elif type=="xyz":
            self.ssroot.loadFromXYZFile(filename)
        self.ss=self.ssroot.getRoot();
        self.glc.setSpinSys(self.ss)
        self.updateSpinTree()

    def saveToFile(self,filename):
        #self.ssroot.setRoot(self.ss);
        self.ssroot.saveToFile(filename)   

    def onSaveAs(self,e):
        fd=wx.FileDialog(self,style=wx.FD_SAVE, message="Save your Spin System",wildcard="Spin XML files (*.xml)|*.xml|All Files (*.*)|*.*") 
        if(fd.ShowModal()):
            saveToFile(fd.GetPath().encode('latin-1'))

    def onSave(self,e):
        print "Impliment me"

    def onFilled(self,e):
        self.glc.setDrawMode('filled')

    def onWireframe(self,e):
        self.glc.setDrawMode('wireframe')

    def onDisplaySpinPropDialog(self,e):
        """Display the spin property dialog for the selected spin"""
        if (self.glc.hover>=0):
            dialog=SpinPropDialog(self,self.ss,self.res,self.glc.hover)
            dialog.Show()

    def onExit(self,e):
        exit(0)

        
        

class SpinPropDialog(wx.Frame):
    def __init__(self,parent,ss,res,index):
        self.ss=ss;
        self.index=index;
        self.parent=parent;
        self.res=res;
        pre = wx.PreFrame();
        self.res.LoadOnFrame(pre,parent,"SpinDialog")
        self.PostCreate(pre)

    def Show(self):
        wx.Frame.Show(self);




class MyApp(wx.App):

    def OnInit(self):
        self.res = xrc.XmlResource('res/gui.xrc')
        self.ssroot=spinsys.SpinsysXMLRoot()
        self.filename=""
        self.filepath=""

        self.rootFrame=RootFrame(self.res,self.ssroot);
        self.rootFrame.Show()

        return True


if __name__ == '__main__':
    app = MyApp(False)
    app.MainLoop()
        
