xcopy ..\distrib\*.* ..\distrib\Backup /a /e /y /i
xcopy D:\Distribution\KSRX\*.* D:\Distribution\Backup\KSRX /a /e /y /i

xcopy ..\include\*.* ..\distrib\include /a /e /y /i
xcopy ..\docs\*.* ..\distrib\docs /a /e /y /i
xcopy ..\build\Debug\*.lib ..\distrib\lib /a /e /y /i
xcopy ..\build\Debug\*.pdb ..\distrib\lib /a /e /y /i
xcopy ..\build\Debug_MT_DLL\*.lib ..\distrib\lib /a /e /y /i
xcopy ..\build\Debug_MT_DLL\*.pdb ..\distrib\lib /a /e /y /i
xcopy ..\build\Release\*.lib ..\distrib\lib /a /e /y /i
xcopy ..\build\Release\*.pdb ..\distrib\lib /a /e /y /i
xcopy ..\build\Release_MT_DLL\*.lib ..\distrib\lib /a /e /y /i
xcopy ..\build\Release_MT_DLL\*.pdb ..\distrib\lib /a /e /y /i
xcopy ..\src\*.* ..\distrib\src /a /e /y /i
xcopy ..\msvc6-proj\*.* ..\distrib\msvc6-proj /a /e /y /i
xcopy ..\msvc7-proj\*.* ..\distrib\msvc7-proj /a /e /y /i