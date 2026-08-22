@echo off
setlocal

rem ============================================================
rem FCN - Compilazione Markdown -> PDF con Pandoc + XeLaTeX
rem Uso:
rem     _FCN_Make_Doc.bat FCN_Ch01-Condizionamento-e-stabilita-v0.1.md
rem
rem Il PDF viene creato nella stessa directory del Markdown.
rem La pushd rende affidabili i percorsi relativi, per esempio:
rem     figures/errore-operazione.pdf
rem ============================================================

if "%~1"=="" (
    echo Uso: %~nx0 NOMEFILE.md
    exit /b 2
)

if not exist "%~1" (
    echo ERRORE: file non trovato: "%~1"
    exit /b 3
)

set "SRC=%~nx1"
set "OUT=%~n1.pdf"
set "SRCDIR=%~dp1"

pushd "%SRCDIR%" || exit /b 4

pandoc "%SRC%" ^
    -o "%OUT%" ^
    --pdf-engine=xelatex ^
    --from=markdown+tex_math_single_backslash ^
    -V geometry:"top=1.6cm, bottom=1.6cm, left=2cm, right=2cm"

set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
    echo.
    echo ERRORE Pandoc/XeLaTeX - codice %RC%
    popd
    exit /b %RC%
)

echo.
echo Creato: "%SRCDIR%%OUT%"

popd
endlocal
