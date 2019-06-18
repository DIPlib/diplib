;NSIS Modern User Interface
;Start Menu Folder Selection Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "DIPlib 3"
  OutFile "DIPlib3_installer.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES64\DIPlib 3"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\DIPlib 3" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;   !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\DIPlib 3" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "!DIPlib" SecDIPlib

  SetOutPath "$INSTDIR"
  
  File /r /x "cmake" /x "__pycache__" "dip\*.*"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\DIPlib 3" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
     
  	; Write dipstart.m
	FileOpen $0 "dipstart.m" w
	FileWrite $0 "addpath('$INSTDIR\share\DIPimage');$\r$\n"
	FileWrite $0 "setenv('PATH', ['$INSTDIR\lib',';',getenv('PATH')]);$\r$\n"
	FileClose $0

    ; Write dipstart.py
    FileOpen $1 "dipstart.py" w
    FileWrite $1 "import sys$\r$\n"
    FileWrite $1 "sys.path.append('$INSTDIR\lib')$\r$\n"
    FileWrite $1 "import PyDIP as dip$\r$\n"
    FileClose $1

	; Write instruction.txt
	FileOpen $2 "instruction.md" w
    FileWrite $2 "# Short usage instruction$\r$\n$\n"
    FileWrite $2 "## MATLAB$\r$\n$\n"
	FileWrite $2 "These instructions explain how to start DIPimage in MATLAB:$\r$\n$\n"
	FileWrite $2 "1. start MATLAB$\r$\n"
	FileWrite $2 "2. execute the command `run('$INSTDIR\dipstart.m')`$\r$\n$\n"
	FileWrite $2 "Optionally, you can create or modify the startup.m file in$\r$\n"
	FileWrite $2 "the default working directory of MATLAB to run `dipstart.m`$\r$\n"
	FileWrite $2 "automatically at the beginning of every session.$\r$\n$\n"
    FileWrite $2 "## Python$\r$\n$\n"
    FileWrite $2 "Copy the content of `$INSTDIR\dipstart.py` to the$\r$\n"
    FileWrite $2 "start of your Python script$\r$\n$\n"
	FileWrite $2 "18/6/2019$\r$\n"
	FileClose $2

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\DIPlib instruction.lnk" "$INSTDIR\instruction.md"

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Images" SecImages

    SetOutPath "$INSTDIR\images"

    File /r "images\*.*"

  ;Store installation folder
  WriteRegStr HKCU "Software\DIPlib 3" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

  	; Write dipstart.m
	FileOpen $3 "dipstart.m" a
	FileSeek $3 0 END
	FileWrite $3 "dipsetpref('ImageFilePath', '$INSTDIR\images');$\r$\n"
	FileClose $3

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDIPlib ${LANG_ENGLISH} "DIPlib libraries, header files and Python and MATLAB interface"
    LangString DESC_SecImages ${LANG_ENGLISH} "DIPlib Images for reference and testing"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDIPlib} $(DESC_SecDIPlib)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecImages} $(DESC_SecImages)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
  Delete "$SMPROGRAMS\$StartMenuFolder\DIPlib instruction.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  DeleteRegKey /ifempty HKCU "Software\DIPlib 3"

SectionEnd