del packages\com.nando.nando\data\*
copy ..\bin\* packages\com.nando.nando\data\
c:\Qt\5.12.3\mingw73_32\bin\windeployqt.exe packages\com.nando.nando\data\nando.exe
c:\Qt\Tools\QtInstallerFramework\3.0\bin\binarycreator.exe -c config/config.xml -p packages NANDO_Installer.exe
