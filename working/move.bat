for %%a IN (.\*.obj) do call "..\bin\Release\colmesh.exe" %%a
for %%a IN (.\meshes\*.irrmesh) do call "..\assets\irrb\imeshcvt.exe" -i %%a -o meshes\%%~na.irrbmesh
for %%a IN (.\*.irr) do move /Y %%a levels\%%~na 
for %%a IN (.\*.col) do move /Y %%a levels\%%~na 

del *.obj
del irrb.log
del meshes\*.irrmesh
