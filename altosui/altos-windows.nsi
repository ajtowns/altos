!addplugindir Instdrv/NSIS/Plugins
; Definitions for Java 1.6 Detection
!define JRE_VERSION "1.6"
!define JRE_URL "http://javadl.sun.com/webapps/download/AutoDL?BundleId=52247&/jre-6u27-windows-i586-p.exe"
!define PRODUCT_NAME "Altus Metrum Windows Software"

Name "Altus Metrum Installer"

; Default install directory
InstallDir "$PROGRAMFILES\AltusMetrum"

; Tell the installer where to re-install a new version
InstallDirRegKey HKLM "Software\AltusMetrum" "Install_Dir"

LicenseText "GNU General Public License Version 2"
LicenseData "../COPYING"

; Need admin privs for Vista or Win7
RequestExecutionLevel admin

ShowInstDetails Show

ComponentText "Altus Metrum Software and Driver Installer"

Function GetJRE
        MessageBox MB_OK "${PRODUCT_NAME} uses Java ${JRE_VERSION} 32-bit, it will now \
                         be downloaded and installed"

        StrCpy $2 "$TEMP\Java Runtime Environment.exe"
        nsisdl::download /TIMEOUT=30000 ${JRE_URL} $2
        Pop $R0 ;Get the return value
                StrCmp $R0 "success" +3
                MessageBox MB_OK "Download failed: $R0"
                Quit
        ExecWait $2
        Delete $2
FunctionEnd


Function DetectJRE
  ReadRegStr $2 HKLM "SOFTWARE\JavaSoft\Java Runtime Environment" \
             "CurrentVersion"
  StrCmp $2 ${JRE_VERSION} done

  Call GetJRE

  done:
FunctionEnd

; Pages to present

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

; And the stuff to install

Section "Install Driver" InstDriver

	InstDrv::InitDriverSetup /NOUNLOAD {4D36E96D-E325-11CE-BFC1-08002BE10318} AltusMetrumSerial
	Pop $0
	DetailPrint "InitDriverSetup: $0"
	InstDrv::DeleteOemInfFiles /NOUNLOAD
	InstDrv::CreateDevice /NOUNLOAD

	SetOutPath $TEMP
	File "../telemetrum.inf"
	InstDrv::InstallDriver /NOUNLOAD "$TEMP\telemetrum.inf"

	SetOutPath $INSTDIR
	File "../telemetrum.inf"

	SetOutPath $WINDIR\Inf
	File "../telemetrum.inf"

SectionEnd

Section "AltosUI Application"
	Call DetectJRE

	SetOutPath $INSTDIR

	File "altosui-fat.jar"
	File "cmudict04.jar"
	File "cmulex.jar"
	File "cmu_time_awb.jar"
	File "cmutimelex.jar"
	File "cmu_us_kal.jar"
	File "en_us.jar"
	File "freetts.jar"
	File "jfreechart.jar"
	File "jcommon.jar"

	File "*.dll"

	File "../icon/*.ico"

	CreateShortCut "$SMPROGRAMS\AltusMetrum.lnk" "$SYSDIR\javaw.exe" "-jar altosui-fat.jar" "$INSTDIR\altus-metrum.ico"
SectionEnd

Section "AltosUI Desktop Shortcut"
	CreateShortCut "$DESKTOP\AltusMetrum.lnk" "$INSTDIR\altosui-fat.jar"  "" "$INSTDIR\altus-metrum.ico"
SectionEnd

Section "TeleMetrum and TeleDongle Firmware"

	SetOutPath $INSTDIR

	File "../src/telemetrum-v1.0/telemetrum-v1.0-${VERSION}.ihx"
	File "../src/telemetrum-v1.1/telemetrum-v1.1-${VERSION}.ihx"
	File "../src/telemini-v1.0/telemini-v1.0-${VERSION}.ihx"
	File "../src/teledongle-v0.2/teledongle-v0.2-${VERSION}.ihx"

SectionEnd

Section "Documentation"

	SetOutPath $INSTDIR

	File "../doc/altusmetrum.pdf"
	File "../doc/altos.pdf"
	File "../doc/telemetry.pdf"
	File "../doc/telemetrum-outline.pdf"
SectionEnd

Section "Uninstaller"

	; Deal with the uninstaller

	SetOutPath $INSTDIR

	; Write the install path to the registry
	WriteRegStr HKLM SOFTWARE\AltusMetrum "Install_Dir" "$INSTDIR"

	; Write the uninstall keys for windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AltusMetrum" "DisplayName" "Altus Metrum"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AltusMetrum" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AltusMetrum" "NoModify" "1"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AltusMetrum" "NoRepair" "1"

	WriteUninstaller "uninstall.exe"
SectionEnd

Section "Uninstall"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AltusMetrum"
	DeleteRegKey HKLM "Software\AltusMetrum"

	Delete "$INSTDIR\*.*"
	RMDir "$INSTDIR"

	; Remove devices
	InstDrv::InitDriverSetup /NOUNLOAD {4D36E96D-E325-11CE-BFC1-08002BE10318} AltusMetrumSerial
	InstDrv::DeleteOemInfFiles /NOUNLOAD
	InstDrv::RemoveAllDevices

	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\AltusMetrum.lnk"
	Delete "$DESKTOP\AltusMetrum.lnk"
SectionEnd
