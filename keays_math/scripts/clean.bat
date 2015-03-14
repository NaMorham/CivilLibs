cd ..\build

del /Q DebugStatic
rmdir DebugStatic
del /Q /S ReleaseStatic
rmdir ReleaseStatic
del /Q Debug
rmdir Debug
del /Q /S Release
rmdir Release

cd ..\lib
del /Q /S *.lib

cd ..\docs
del /Q /S html
rmdir html
del /Q /S latex
rmdir latex
del /Q keays_math.log
del /Q keays_math.tag


cd ..\msvc-project
del keays_math.plg
del keays_math.opt
del keays_math.ncb
del keays_math.mak
del keays_math.dep

cd ..\msvc7-project
del keays_math.ncb
attrib -h keays_math.suo
del keays_math.suo

cd ..\scripts