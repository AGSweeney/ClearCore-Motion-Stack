#define MyAppName "MotionBench"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Adam G. Sweeney"
#define MyAppURL "https://github.com/agsweeney1972/ClearLinkMonitor"
#define MyReleaseDir "..\\build-vs\\Release"
#define MySetupIcon "..\\assets\\icons\\motionbench-app.ico"

[Setup]
AppId={{3C39F493-84DA-4D82-BD2B-4A294B3C4E1B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\MotionBench
DefaultGroupName=MotionBench
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=output
OutputBaseFilename=MotionBench-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
SetupIconFile={#MySetupIcon}
UninstallDisplayIcon={app}\MotionBench.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"

[Files]
Source: "{#MyReleaseDir}\\MotionBench.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyReleaseDir}\\BoardModeTool.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyReleaseDir}\\*.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "{#MyReleaseDir}\\generic\\*"; DestDir: "{app}\\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\iconengines\\*"; DestDir: "{app}\\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\imageformats\\*"; DestDir: "{app}\\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\networkinformation\\*"; DestDir: "{app}\\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\platforms\\*"; DestDir: "{app}\\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\qml\\*"; DestDir: "{app}\\qml"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\tls\\*"; DestDir: "{app}\\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReleaseDir}\\translations\\*"; DestDir: "{app}\\translations"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; Place shortcuts under Programs\MotionBench\ (named folder), not loose in Programs root.
Name: "{commonprograms}\MotionBench\{#MyAppName}"; Filename: "{app}\MotionBench.exe"
Name: "{commonprograms}\MotionBench\Board Mode Tool"; Filename: "{app}\BoardModeTool.exe"
Name: "{autodesktop}\MotionBench"; Filename: "{app}\MotionBench.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\MotionBench.exe"; Description: "Launch MotionBench"; Flags: nowait postinstall skipifsilent
