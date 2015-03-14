xcopy D:\Distribution\KSR\*.* D:\Distribution\Backup\KSR /a /e /y /i
xcopy D:\Distribution\KSRX\*.* D:\Distribution\Backup\KSRX /a /e /y /i

copy D:\KSR\KSR\Debug\*.lib D:\Distribution\KSR\lib
copy D:\KSR\KSR\Debug_MT_DLL\*.lib D:\Distribution\KSR\lib
copy D:\KSR\KSR\Release\*.lib D:\Distribution\KSR\lib
copy D:\KSR\KSR\Release_MT_DLL\*.lib D:\Distribution\KSR\lib

copy D:\KSR\KSR\Debug\*.pdb D:\Distribution\KSR\lib
copy D:\KSR\KSR\Debug_MT_DLL\*.pdb D:\Distribution\KSR\lib

copy D:\KSR\KSRX\Debug\*.lib D:\Distribution\KSRX\lib
copy D:\KSR\KSRX\Debug_MT_DLL\*.lib D:\Distribution\KSRX\lib
copy D:\KSR\KSRX\Release\*.lib D:\Distribution\KSRX\lib
copy D:\KSR\KSRX\Release_MT_DLL\*.lib D:\Distribution\KSRX\lib

copy D:\KSR\KSRX\Debug\*.pdb D:\Distribution\KSRX\lib
copy D:\KSR\KSRX\Debug_MT_DLL\*.pdb D:\Distribution\KSRX\lib

copy D:\KSR\KSR\*.h D:\Distribution\KSR\include
copy D:\KSR\KSR\*.cpp D:\Distribution\KSR\src

copy D:\KSR\KSRX\*.h D:\Distribution\KSRX\include
copy D:\KSR\KSRX\*.cpp D:\Distribution\KSRX\src

copy D:\KSR\KSR\*.sln D:\Distribution\KSR\src
copy D:\KSR\KSR\*.vcproj D:\Distribution\KSR\src
copy D:\KSR\KSRX\*.vcproj D:\Distribution\KSRX\src