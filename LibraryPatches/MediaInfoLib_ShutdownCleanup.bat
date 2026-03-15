@echo off
setlocal
pushd "%~dp0.."
git apply "LibraryPatches\MediaInfoLib_ShutdownCleanup.patch"
popd
endlocal
