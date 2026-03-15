@echo off
setlocal
pushd "%~dp0.."
git apply "LibraryPatches\ResizableLib_eMuleAI.patch"
popd
endlocal
