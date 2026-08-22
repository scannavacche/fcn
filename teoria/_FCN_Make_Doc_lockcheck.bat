@echo off
setlocal

rem ============================================================
rem FCN - Compila un Markdown in PDF con Pandoc + XeLaTeX
rem Uso:
rem     _FCN_Make_Doc.bat FCN_Ch01-Condizionamento-e-stabilita-v0.1.md
rem
rem In cmd.exe si puo' usare TAB per completare il nome del file.
rem ============================================================

if "%~1"=="" (
    echo.
    echo Uso: %~nx0 FILE.md
    echo.
    exit /b 2
)

if not exist "%~1" (
    echo.
    echo ERRORE: sorgente non trovato:
    echo   "%~1"
    echo.
    exit /b 3
)

set "SRC=%~nx1"
set "SRCDIR=%~dp1"
set "OUT=%~n1.pdf"

pushd "%SRCDIR%" || (
    echo.
    echo ERRORE: impossibile accedere alla directory:
    echo   "%SRCDIR%"
    echo.
    exit /b 4
)

rem ------------------------------------------------------------
rem Preflight: se il PDF esiste, controlla che non sia impegnato.
rem FileShare.None fallisce se Acrobat o un altro processo lo
rem tiene aperto in modo incompatibile con una sovrascrittura.
rem ------------------------------------------------------------
if exist "%OUT%" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
      "$p = [IO.Path]::GetFullPath('%OUT%');" ^
      "try {" ^
      "  $s = [IO.File]::Open($p,[IO.FileMode]::Open,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None);" ^
      "  $s.Close(); exit 0" ^
      "} catch {" ^
      "  exit 1" ^
      "}"

    if errorlevel 1 (
        echo.
        echo ============================================================
        echo ERRORE: PDF BLOCCATO O IMPEGNATO
        echo   "%OUT%"
        echo.
        echo Chiudilo in Acrobat ^(o nel programma che lo sta usando^)
        echo e rilancia la compilazione.
        echo ============================================================
        echo.
        popd
        exit /b 10
    )

    rem Elimina il vecchio PDF prima di compilare.
    rem Cosi', se Pandoc fallisce, non resta in giro una copia vecchia
    rem che possa essere scambiata per il documento appena generato.
    del /q "%OUT%" >nul 2>&1

    if exist "%OUT%" (
        echo.
        echo ERRORE: impossibile eliminare il vecchio PDF:
        echo   "%OUT%"
        echo.
        popd
        exit /b 11
    )
)

echo.
echo Compilo:
echo   "%SRC%"
echo.

pandoc "%SRC%" ^
    -o "%OUT%" ^
    --pdf-engine=xelatex ^
    --from=markdown+tex_math_single_backslash ^
    -V geometry:"top=1.6cm, bottom=1.6cm, left=2cm, right=2cm"

set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
    echo.
    echo ============================================================
    echo ERRORE DI COMPILAZIONE - codice %RC%
    echo Nessun PDF nuovo e' stato prodotto.
    echo ============================================================
    echo.
    popd
    exit /b %RC%
)

if not exist "%OUT%" (
    echo.
    echo ERRORE: Pandoc ha terminato senza creare:
    echo   "%OUT%"
    echo.
    popd
    exit /b 12
)

echo.
echo ============================================================
echo OK - PDF creato:
echo   "%SRCDIR%%OUT%"
echo ============================================================
echo.

popd
endlocal
exit /b 0
