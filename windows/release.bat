del /Q /S packages\com.nando.nando\data\*
md packages\com.nando.nando\data\
copy ..\bin\* packages\com.nando.nando\data\
copy C:\boost\lib\libboost_thread-mgw8-mt-x64-1_75.dll packages\com.nando.nando\data\
copy C:\boost\lib\libboost_system-mgw8-mt-x64-1_75.dll packages\com.nando.nando\data\
c:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe packages\com.nando.nando\data\nando.exe
c:\Qt\QtIFW\bin\binarycreator.exe -c config/config.xml -p packages NANDO_Installer.exe
