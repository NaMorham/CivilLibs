REM -------------------
REM keays_types cleanup
REM -------------------
cd ..\build
del /q DebugStatic
rmdir DebugStatic
del /q ReleaseStatic
rmdir ReleaseStatic
del /q Release
rmdir Release
del /q Debug
rmdir Debug

cd ..\lib
del /q *

cd ..\msvc-project
del keays_types.ncb
del keays_types.opt
del keays_types.plg
del keays_types.mak
del keays_types.dep

cd ..\msvc7-project
del keays_types.ncb
attrib -h keays_types.suo
del keays_types.suo

cd ..\scripts