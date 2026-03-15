@echo off
setlocal
pushd "%~dp0.."
git apply "LibraryPatches\MbedTLS_threading_alt_add.patch"
popd
endlocal
