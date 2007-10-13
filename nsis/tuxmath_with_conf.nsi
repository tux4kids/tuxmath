# vim: noai et ts=4 tw=0
# with a few tiny modifications by Phil Harper(philh@theopencd.org)
# modified for tuxmath by Yves Combe (yves@ycombe.net)

!define PKG_VERSION "1.5.6"
!define PKG_PREFIX  "tuxmath"

!define APP_PREFIX  "TuxMath"
!define APP_EXE     "${APP_PREFIX}.exe"
!define APP_NAME    "Tux of Math Command"

!define CONF_EXE     "tuxmath_config.exe"
!define CONF_DIR     "TuxMathConfig"

OutFile     "${PKG_PREFIX}-${PKG_VERSION}-with-config-win32-installer.exe"
Name        "${APP_NAME}"
Caption     ""
CRCCheck    on
WindowIcon  off
BGGradient  off

# Default to not silent
SilentInstall   normal
SilentUnInstall normal

# Various default text options
MiscButtonText
InstallButtonText
FileErrorText

# Default installation dir and registry key of install directory
InstallDir  "$PROGRAMFILES\${APP_PREFIX}"
InstallDirRegKey HKLM SOFTWARE\${APP_PREFIX} "Install_Dir"

# Licence text
LicenseText "You must agree to this license before installing ${APP_NAME}"
LicenseData "docs\COPYING.txt"

# Directory browsing
# DirShow           show
ComponentText       "This will install ${APP_NAME} on your computer. Select which optional things you want installed."
DirText             "Choose a directory to install ${APP_NAME} in to:"
AllowRootDirInstall false

# Install page stuff
InstProgressFlags   smooth
AutoCloseWindow     true

Section
  SetOutPath $INSTDIR
  File "mingw32\${APP_EXE}"
  File "mingw32\*.dll"
  SetOutPath $INSTDIR\data
  File /r "data\*.*"
  SetOutPath $INSTDIR\docs
  File "docs\COPYING.txt"
  SetOutPath $INSTDIR\${CONF_DIR}
  File "mingw32\TuxMathConfig\*.*"

  WriteRegStr HKLM SOFTWARE\${APP_PREFIX} "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}" "DisplayName" "${APP_NAME} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd


Section "Start Menu Shortcuts"
  SetShellVarContext all
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME} (Full Screen).lnk" "$INSTDIR\${APP_EXE}" "-f" "$INSTDIR\${APP_EXE}" 0 "" "" "Start TuxMath in Fullscreen mode"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME} (Windowed).lnk" "$INSTDIR\${APP_EXE}" "-w" "$INSTDIR\${APP_EXE}" 0 "" "" "Start TuxMath in a Window"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Configure TuxMath.lnk" "$INSTDIR\${CONF_DIR}\${CONF_EXE}" "-w" "$INSTDIR\${CONF_DIR}\${CONF_EXE}" 0 "" "" "Configure TuxMath"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 "" "" "Remove Tux of Math Command"
SectionEnd


Section "Desktop Shortcut"
  SetShellVarContext all
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0  "" "" "Run Tux of Math Command"
SectionEnd

;Function .onInstSuccess
;  BringToFront
;  MessageBox MB_YESNO|MB_ICONQUESTION \
;             "${APP_NAME} was installed. Would you like to run ${APP_NAME} now ?" \
;             IDNO NoExec
;    Exec '$INSTDIR\${APP_EXE}'
;  NoExec:
;FunctionEnd

; uninstall stuff

UninstallText "This will uninstall ${APP_NAME}. Hit 'Uninstall' to continue."

; special uninstall section.
Section "Uninstall"
  SetShellVarContext all
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}"
  DeleteRegKey HKLM SOFTWARE\${APP_PREFIX}

  RMDir  /r "$INSTDIR\data"
  RMDir  /r "$INSTDIR\docs"
  RMDir  /r "$INSTDIR\${CONF_DIR}"
  Delete    "$INSTDIR\*.*"

  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\*.*"
  RMDir  "$SMPROGRAMS\${APP_NAME}"
SectionEnd


