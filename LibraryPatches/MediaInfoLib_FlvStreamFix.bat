@echo off
setlocal
pushd "%~dp0.."
git apply "LibraryPatches\MediaInfoLib_FlvStreamFix.patch"
popd
endlocal
