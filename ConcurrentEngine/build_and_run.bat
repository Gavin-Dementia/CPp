@echo off
setlocal

:: 進入專案目錄
cd /d %~dp0

:: 移除舊 build 目錄
echo [INFO] Cleaning old build directory...
rmdir /s /q build 2>nul

:: 建立 build 目錄
mkdir build
cd build

:: 產生 Makefile
echo [INFO] Generating Makefiles with CMake...
cmake .. -G "MinGW Makefiles"

:: 檢查 cmake 是否成功
if errorlevel 1 (
    echo [ERROR] CMake generation failed.
    pause
    exit /b 1
)

:: 編譯
echo [INFO] Building project with mingw32-make...
mingw32-make

:: 檢查 make 是否成功
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

:: 執行 dag_test.exe
echo [INFO] Running dag_test.exe...
dag_test.exe

pause
endlocal
