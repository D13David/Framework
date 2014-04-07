@echo off

cd ../../Data/Textures

for /r %%F in (*.tga) do (
  "C:\Program Files\NVIDIA Corporation\NVIDIA Texture Tools 2\bin\nvcompress.exe" "%%~dpnxF"
)

pause