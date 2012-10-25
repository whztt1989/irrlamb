for %%a IN (..\scenes\*.obj) do call colmesh.exe %%a
xcopy /Y ..\scenes\*.col ..\..\working\collision
del ..\scenes\*.obj
del ..\scenes\*.mtl
del ..\scenes\*.col
