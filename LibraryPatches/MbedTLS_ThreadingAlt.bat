@echo off
setlocal
pushd "%~dp0.."
git apply "LibraryPatches\MbedTLS_ThreadingAlt.patch"
popd
endlocal
