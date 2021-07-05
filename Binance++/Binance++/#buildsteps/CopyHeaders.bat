REM %1 : Import folder for headers
REM %2 : Export folder for headers

IF EXIST %2 rmdir %2 /s /q
mkdir %2

robocopy %1 %2 *.h /IT /IM /S
if %ERRORLEVEL% GEQ 8 goto failed

goto eof
:failed
exit %ERRORLEVEL%

:eof
exit /B 0