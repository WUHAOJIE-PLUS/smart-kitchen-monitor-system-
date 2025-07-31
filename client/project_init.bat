@echo off
setlocal

cd /d "%~dp0"

REM === 获取当前目录名 ===
for %%i in (.) do set "CUR_DIR=%%~nxi"

REM === 重命名 .uvprojx 文件 ===
for %%f in (*.uvprojx) do (
    if not "%%~nxf"=="%CUR_DIR%.uvprojx" (
        ren "%%~nxf" "%CUR_DIR%.uvprojx"
    )
)

REM === 重命名 .uvoptx 文件（只有一个）===
for %%f in (*.uvoptx) do (
    if not "%%~nxf"=="%CUR_DIR%.uvoptx" (
        ren "%%~nxf" "%CUR_DIR%.uvoptx"
    )
)

REM === 删除所有 .uvguix.*（Keil 会重新生成）===
del /q *.uvguix.* >nul 2>nul

REM === 删除 EventRecorderStub.scvd 文件 ===
del /q EventRecorderStub.scvd >nul 2>nul

REM === 打开工程文件 ===
start "" "%CUR_DIR%.uvprojx"

REM === 精确复制当前目录名到剪贴板（无换行、无空格）===
powershell -Command "Set-Clipboard -Value '%CUR_DIR%'"
