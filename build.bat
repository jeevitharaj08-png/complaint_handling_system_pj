@echo off
rem ===========================================================================
rem Build script for Complaint Handling System (Windows / MinGW GCC)
rem ===========================================================================

echo ===================================================
echo  Building Complaint Handling System (C11)
echo ===================================================

where gcc >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] GCC compiler was not found in your PATH.
    echo Please ensure MinGW-w64 or MSYS2 GCC is installed and added to PATH.
    pause
    exit /b 1
)

if not exist bin mkdir bin
if not exist data mkdir data

echo Compiling main application (bin\complaint_system.exe)...
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src\*.c -o bin\complaint_system.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Application compilation failed.
    pause
    exit /b %ERRORLEVEL%
)

echo Compiling automated test runner (bin\test_runner.exe)...
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src\utils.c src\customer.c src\agent.c src\complaint.c src\search.c src\report.c src\storage.c src\audit.c tests\test_runner.c -o bin\test_runner.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Test suite compilation failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] Build completed successfully!
echo   Main App   : bin\complaint_system.exe
echo   Test Suite : bin\test_runner.exe
echo ===================================================
