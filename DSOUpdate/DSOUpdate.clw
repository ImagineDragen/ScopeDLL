; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDSOUpdateDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "DSOUpdate.h"

ClassCount=3
Class1=CDSOUpdateApp
Class2=CDSOUpdateDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_DSOUPDATE_DIALOG
Resource4=IDD_ABOUTBOX (English (U.S.))
Resource5=IDD_DSOUPDATE_DIALOG (English (U.S.))

[CLS:CDSOUpdateApp]
Type=0
HeaderFile=DSOUpdate.h
ImplementationFile=DSOUpdate.cpp
Filter=N

[CLS:CDSOUpdateDlg]
Type=0
HeaderFile=DSOUpdateDlg.h
ImplementationFile=DSOUpdateDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_BTN_UPDATE

[CLS:CAboutDlg]
Type=0
HeaderFile=DSOUpdateDlg.h
ImplementationFile=DSOUpdateDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg


[DLG:IDD_DSOUPDATE_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CDSOUpdateDlg

[DLG:IDD_DSOUPDATE_DIALOG (English (U.S.))]
Type=1
Class=CDSOUpdateDlg
ControlCount=8
Control1=IDC_STC_PATH,static,1342308352
Control2=IDC_STC_UPDATE,button,1342177287
Control3=IDC_EDIT_PATH,edit,1350631552
Control4=IDC_BTN_BROWSER,button,1342242816
Control5=IDC_BTN_UPDATE,button,1342242816
Control6=IDC_BTN_READ,button,1073807360
Control7=IDC_PRO,msctls_progress32,1350565888
Control8=IDC_STC_INFO,static,1342308352

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

