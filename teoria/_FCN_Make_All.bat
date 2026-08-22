@echo off
setlocal

call _FCN_Make_Doc_lockcheck.bat FCN_Guida-approfondimenti_Fonti_v0.2.md
if errorlevel 1 exit /b %errorlevel%


rem call _FCN_Make_Doc_lockcheck.bat FCN_Ch01-Condizionamento-e-stabilita-v0.1.md
rem if errorlevel 1 exit /b %errorlevel%

rem Aggiungere i capitoli man mano:
rem call _FCN_Make_Doc_lockcheck.bat FCN_Ch02-Norme-v0.1.md
rem if errorlevel 1 exit /b %errorlevel%

rem call _FCN_Make_Doc_lockcheck.bat FCN_Ch03-Sistemi-lineari-v0.1.md
rem if errorlevel 1 exit /b %errorlevel%

echo.
echo Tutti i PDF richiesti sono stati compilati.
echo.
endlocal
