@if not "%VCINSTALLDIR%"=="" goto build
@set VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio .NET
@set PATH=%VCINSTALLDIR%\Common7\IDE;%PATH%

:build
@rem devenv /build release /project dfs utils\dfs\dfs.sln
@rem devenv /build release /project mkfloppy utils\mkfloppy\mkfloppy.sln
@rem devenv /build release /project mkpart utils\mkpart\mkpart.sln
@rem devenv /build release /project dbggw utils\dbggw\dbggw.sln
@rem devenv /build sanos /project image build\sanos.sln
nmake all
