@if not "%VCINSTALLDIR%"=="" goto build
@set VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio .NET
@set PATH=%VCINSTALLDIR%\Common7\IDE;%PATH%

:build
devenv /build release /project dfs utils\dfs\dfs.sln
devenv /build release /project mkfloppy utils\mkfloppy\mkfloppy.sln
devenv /build release /project mkpart utils\mkpart\mkpart.sln
devenv /build release /project dbggw utils\dbggw\dbggw.sln
devenv /build sanos /project image build\sanos.sln
