@echo off
setlocal enabledelayedexpansion

:: Get the directory where this script lives
set "SCRIPT_DIR=%~dp0"
:: Remove trailing backslash
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

:: Convert Windows path to WSL path
for /f "delims=" %%i in ('wsl wslpath -a "%SCRIPT_DIR%"') do set "WSL_DIR=%%i"

echo ============================================
echo   Polygon Simplification - Build and Run
echo ============================================
echo.

:: Step 1: Build the executable
:BUILD
echo [1/2] Building simplify executable via WSL...
echo.
wsl bash -c "cd '%WSL_DIR%' && make clean && make"
if !errorlevel! neq 0 (
    echo.
    echo ERROR: Build failed! Check the errors above.
    echo.
    set /p "RETRY=Retry build? [y/n]: "
    if /i "!RETRY!"=="y" goto :BUILD
    echo.
    echo WARNING: Continuing without a successful build. Tests may fail.
    echo.
    goto :SKIP_BUILD_MSG
)
echo.
echo Build successful!
echo.
:SKIP_BUILD_MSG

:: Create output folder if it doesn't exist
if not exist "%SCRIPT_DIR%\output" mkdir "%SCRIPT_DIR%\output"

:: Step 2: Discover test cases and parse README for recommended targets
set "SAMPLE_COUNT=0"
for %%f in ("%SCRIPT_DIR%\sample_test_cases\*.csv") do (
    set /a SAMPLE_COUNT+=1
    set "SAMPLE_!SAMPLE_COUNT!=%%~nxf"
    set "SAMPLE_REC_!SAMPLE_COUNT!="
    for /f "tokens=*" %%r in ('wsl bash -c "grep '%%~nxf' '%WSL_DIR%/sample_test_cases/README.md' 2>/dev/null" ^| wsl bash -c "sed -n 's/.*| *\`%%~nxf\` *| *\([0-9]*\) *|.*/\1/p'"') do (
        set "SAMPLE_REC_!SAMPLE_COUNT!=%%r"
    )
)

set "MY_COUNT=0"
for %%f in ("%SCRIPT_DIR%\my_test_cases\*.csv") do (
    set /a MY_COUNT+=1
    set "MYTEST_!MY_COUNT!=%%~nxf"
    set "MYTEST_REC_!MY_COUNT!="
    if exist "%SCRIPT_DIR%\my_test_cases\README.md" (
        for /f "tokens=*" %%r in ('wsl bash -c "grep '%%~nxf' '%WSL_DIR%/my_test_cases/README.md' 2>/dev/null" ^| wsl bash -c "sed -n 's/.*| *\`%%~nxf\` *| *\([0-9]*\) *|.*/\1/p'"') do (
            set "MYTEST_REC_!MY_COUNT!=%%r"
        )
    )
)

:MENU
echo ============================================
echo   Select Test Cases to Run
echo ============================================
echo.

:: --- Sample Test Cases ---
echo   --- Sample Test Cases ---
if !SAMPLE_COUNT! equ 0 (
    echo   [Nothing found]
    echo.
    goto :MENU_MY
)
for /l %%i in (1,1,!SAMPLE_COUNT!) do (
    set "REC=!SAMPLE_REC_%%i!"
    if defined REC (
        echo   %%i. !SAMPLE_%%i!  -- recommended: !REC! vertices
    ) else (
        echo   %%i. !SAMPLE_%%i!
    )
)
echo.
set /a SAMPLE_ALL_IDX=SAMPLE_COUNT+1
echo   !SAMPLE_ALL_IDX!. Run ALL sample test cases -- uses recommended targets
echo.

:: --- My Test Cases ---
:MENU_MY
echo   --- My Test Cases ---
if !MY_COUNT! equ 0 (
    echo   [Nothing found]
    echo.
    goto :MENU_ALL
)
set /a MY_START=SAMPLE_ALL_IDX+1
for /l %%i in (1,1,!MY_COUNT!) do (
    set /a DISPLAY_IDX=MY_START+%%i-1
    set "REC=!MYTEST_REC_%%i!"
    if defined REC (
        echo   !DISPLAY_IDX!. !MYTEST_%%i!  -- recommended: !REC! vertices
    ) else (
        echo   !DISPLAY_IDX!. !MYTEST_%%i!
    )
)
echo.
set /a MY_ALL_IDX=MY_START+MY_COUNT
echo   !MY_ALL_IDX!. Run ALL my test cases -- uses recommended targets if available
echo.

:: --- Run ALL option ---
:MENU_ALL
set "ALL_IDX="
if !SAMPLE_COUNT! gtr 0 if !MY_COUNT! gtr 0 (
    set /a ALL_IDX=MY_ALL_IDX+1
    echo   --- All ---
    echo   !ALL_IDX!. Run ALL test cases -- uses recommended targets
    echo.
)

if !SAMPLE_COUNT! equ 0 if !MY_COUNT! equ 0 (
    echo   No test cases found in either folder.
    echo   Add .csv files to sample_test_cases or my_test_cases and try again.
    pause
    goto :MENU
)

echo   0. Exit
echo.
echo ============================================
set /p "CHOICE=Enter your choice: "

if "!CHOICE!"=="0" goto :END

:: Validate choice is a number
set "VALID="
for /f "delims=0123456789" %%i in ("!CHOICE!") do set "VALID=%%i"
if defined VALID (
    echo Invalid choice. Please enter a number.
    echo.
    goto :MENU
)

:: --- Check "Run ALL" options first (no target prompt) ---

:: Run ALL test cases
if defined ALL_IDX (
    if !CHOICE! equ !ALL_IDX! goto :RUN_ALL
)

:: Run ALL sample test cases
if !SAMPLE_COUNT! gtr 0 (
    if !CHOICE! equ !SAMPLE_ALL_IDX! goto :RUN_ALL_SAMPLES
)

:: Run ALL my test cases
if !MY_COUNT! gtr 0 (
    if !CHOICE! equ !MY_ALL_IDX! goto :RUN_ALL_MY
)

:: --- Individual test case selection ---
set "SELECTED_FILE="
set "SELECTED_FOLDER="
set "SELECTED_REC="

:: Check sample test cases
if !SAMPLE_COUNT! gtr 0 if !CHOICE! geq 1 if !CHOICE! leq !SAMPLE_COUNT! (
    set "SELECTED_FILE=!SAMPLE_%CHOICE%!"
    set "SELECTED_FOLDER=sample_test_cases"
    set "SELECTED_REC=!SAMPLE_REC_%CHOICE%!"
)

:: Check my test cases
if not defined SELECTED_FILE if !MY_COUNT! gtr 0 (
    set /a MY_OFFSET=CHOICE-MY_START+1
    if !MY_OFFSET! geq 1 if !MY_OFFSET! leq !MY_COUNT! (
        set "SELECTED_FILE=!MYTEST_%MY_OFFSET%!"
        set "SELECTED_FOLDER=my_test_cases"
        set "SELECTED_REC=!MYTEST_REC_%MY_OFFSET%!"
    )
)

if not defined SELECTED_FILE (
    echo Invalid choice. Please try again.
    echo.
    goto :MENU
)

:: --- Ask for target vertex count ---
echo.
if not defined SELECTED_REC goto :ASK_TARGET_NO_REC

:: Has a recommended target
echo Recommended target for this file: !SELECTED_REC! vertices
set "TARGET="
set /p "TARGET=Enter target vertex count [press Enter for !SELECTED_REC!]: "
if "!TARGET!"=="" set "TARGET=!SELECTED_REC!"
goto :VALIDATE_TARGET

:ASK_TARGET_NO_REC
set "TARGET="
set /p "TARGET=Enter target vertex count: "

:VALIDATE_TARGET
if "!TARGET!"=="" (
    echo No target entered. Please try again.
    echo.
    goto :MENU
)
set "VALID="
for /f "delims=0123456789" %%i in ("!TARGET!") do set "VALID=%%i"
if defined VALID (
    echo Invalid target. Please enter a number.
    echo.
    goto :MENU
)

echo.
call :RUN_SINGLE "!SELECTED_FOLDER!" "!SELECTED_FILE!" "!TARGET!"
goto :DONE

:: ============================================
:: Run all test cases
:: ============================================
:RUN_ALL
echo Running ALL test cases -- using recommended targets...
echo.
set "RUN_SAMPLES=1"
set "RUN_MY=1"
goto :RUN_BATCH

:: ============================================
:: Run all sample test cases
:: ============================================
:RUN_ALL_SAMPLES
echo Running ALL sample test cases -- using recommended targets...
echo.
set "RUN_SAMPLES=1"
set "RUN_MY=0"
goto :RUN_BATCH

:: ============================================
:: Run all my test cases
:: ============================================
:RUN_ALL_MY
echo Running ALL my test cases -- using recommended targets where available...
echo.
set "RUN_SAMPLES=0"
set "RUN_MY=1"
goto :RUN_BATCH

:: ============================================
:: Batch runner - runs samples and/or my tests
:: ============================================
:RUN_BATCH
if "!RUN_SAMPLES!"=="1" (
    for /l %%i in (1,1,!SAMPLE_COUNT!) do (
        set "FILE=!SAMPLE_%%i!"
        set "REC=!SAMPLE_REC_%%i!"
        if defined REC (
            call :RUN_SINGLE "sample_test_cases" "!FILE!" "!REC!"
        ) else (
            echo   Skipping !FILE! - no recommended target found in README
        )
    )
)
if "!RUN_MY!"=="1" (
    for /l %%i in (1,1,!MY_COUNT!) do (
        set "FILE=!MYTEST_%%i!"
        set "REC=!MYTEST_REC_%%i!"
        if defined REC (
            call :RUN_SINGLE "my_test_cases" "!FILE!" "!REC!"
        ) else (
            echo   Skipping !FILE! - no recommended target found in README
        )
    )
)
goto :DONE

:: ============================================
:: Run a single test case
:: Args: %1 = folder, %2 = filename, %3 = target vertices
:: ============================================
:RUN_SINGLE
set "FOLDER=%~1"
set "FILENAME=%~2"
set "TARGET_V=%~3"
set "BASENAME=%FILENAME:.csv=%"

echo --------------------------------------------
echo Running: %FOLDER%/%FILENAME% -- target: %TARGET_V% vertices
echo --------------------------------------------

set "OUT_FILE=%SCRIPT_DIR%\output\%BASENAME%_t%TARGET_V%.txt"

wsl bash -c "cd '%WSL_DIR%' && ./simplify '%FOLDER%/%FILENAME%' %TARGET_V%" > "%OUT_FILE%" 2>&1

if !errorlevel! neq 0 (
    echo   FAILED - check output file for details
) else (
    echo   Done - output saved to: output\%BASENAME%_t%TARGET_V%.txt
)
echo.
goto :eof

:: ============================================
:DONE
echo ============================================
echo   All done! Results are in the output folder.
echo ============================================
echo.
pause
goto :MENU

:END
echo Goodbye!
endlocal
