for %%a IN (.\*.obj) do call "..\assets\collision\colmesh.exe" %%a
for %%a IN (.\*.irr) do move /Y %%a "levels/"%%~na 
for %%a IN (.\*.col) do move /Y %%a "levels/"%%~na 

del *.obj
del irrb.log