
function OnFinish(selproj, selObj)
{
  try
  {
    var projpath = wizard.FindSymbol('PROJECT_PATH');
    var projname = wizard.FindSymbol('PROJECT_NAME');
    var emptyproj = wizard.FindSymbol('EMPTY_PROJ');
    var extproj = wizard.FindSymbol('EXT_PROJ');
    var bootdisk = wizard.FindSymbol('BOOTDISK');
    
    if (extproj)
    {
      // Remove project name from project path
      projpath = projpath.substr(0, projpath.length - projname.length - 1);
      selproj = CreateCustomProject(projname, projpath + '\\build');
    }
    else
    {
      selproj = CreateCustomProject(projname, projpath);
    }

    AddConfig(selproj, projname);
    
    if (!emptyproj)
    {
      var inffile = CreateCustomInfFile('templates');

      if (extproj)
        AddFilesToCustomProj(selproj, projpath + '\\src\\' + projname, inffile);
      else
        AddFilesToCustomProj(selproj, projpath, inffile);

      //PchSettings(selproj);
      inffile.Delete();
    }

    if (bootdisk) 
    {
      var bdproj = CreateBootDiskProject(projpath, projname);
      DTE.Solution.SolutionBuild.BuildDependencies.Item(bdproj.UniqueName).AddProject(selproj.UniqueName)
      bdproj.Object.Save();
    }

    selproj.Object.Save();
  }
  catch (e)
  {
    ReportError(e.description);
    if (e.description.length != 0) SetErrorInfo(e);
    return e.number
  }
}

function CreateCustomProject(projname, projpath)
{
  try
  {
    var projtemplatepath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
    var projtemplate = '';
    projtemplate = projtemplatepath + '\\default.vcproj';

    var solution = DTE.Solution;
    var solutionname = '';
    if (wizard.FindSymbol('CLOSE_SOLUTION'))
    {
      solution.Close();
      solutionname = wizard.FindSymbol('VS_SOLUTION_NAME');
      if (solutionname.length)
      {
        var solutionpath = projpath.substr(0, projpath.length - projname.length);
        solution.Create(solutionpath, solutionname);
      }
    }

    var projnamewithext = '';
    projnamewithext = projname + '.vcproj';

    var target = wizard.FindSymbol('TARGET');
    var prj;
    if (wizard.FindSymbol('WIZARD_TYPE') == vsWizardAddSubProject)
    {
      var prjItem = target.AddFromTemplate(projtemplate, projnamewithext);
      prj = prjItem.SubProject;
    }
    else
    {
      prj = target.AddFromTemplate(projtemplate, projpath, projnamewithext);
    }
    return prj;
  }
  catch (e)
  {
    throw e;
  }
}

function AddConfig(proj, projname)
{
  try
  {
    var prjname = wizard.FindSymbol('PROJECT_NAME');
    var sdkpath = wizard.FindSymbol('SANOS_SDKPATH');
    var isdll = !wizard.FindSymbol('APP_TYPE_USEREXE') && !wizard.FindSymbol('APP_TYPE_USERLIB');
    var islib = wizard.FindSymbol('APP_TYPE_USERLIB');
    var isdrv = wizard.FindSymbol('APP_TYPE_KRNLDRV');
    var iskrnl = wizard.FindSymbol('APP_TYPE_KRNLDRV') || wizard.FindSymbol('APP_TYPE_KRNLMOD');
    var useclib = wizard.FindSymbol('USE_CLIB');
    var defines = wizard.FindSymbol('DEFINES');
    var extproj = wizard.FindSymbol('EXT_PROJ');
    
    var config = proj.Object.Configurations('Debug');

    if (!extproj)
    {
      config.IntermediateDirectory = 'debug';
      config.OutputDirectory = 'debug';
    }
    else
    {
      config.IntermediateDirectory = '..\\dbg\\obj\\' + projname;
      if (islib)
        config.OutputDirectory = '..\\dbg\\lib';
      else
        config.OutputDirectory = '..\\dbg\\bin';
    }
    
    if (isdll) 
      config.ConfigurationType = 2;
    else if (islib)
      config.ConfigurationType = 4;
    
    var cltool = config.Tools('VCCLCompilerTool');
    cltool.Optimization = 0;
    cltool.AdditionalIncludeDirectories = sdkpath + '\\src\\include';
    if (isdll || islib)
    {
      if (iskrnl)
        cltool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB;KERNEL;DEBUG;SANOS';
      else
        cltool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB;DEBUG;SANOS';
    }
    else
      cltool.PreprocessorDefinitions = prjname.toUpperCase() + ';DEBUG;SANOS';

    if (defines != '')
    {
      cltool.PreprocessorDefinitions = cltool.PreprocessorDefinitions + ';' + defines;
    }
      
    cltool.IgnoreStandardIncludePath = true;
    cltool.ExceptionHandling = false;
    cltool.RuntimeLibrary = 0;
    cltool.BufferSecurityCheck = false;
    cltool.UsePrecompiledHeader = 2;
    cltool.PrecompiledHeaderThrough = '';
    cltool.PrecompiledHeaderFile = '$(IntDir)\\' + prjname + '.pch';
    cltool.WarningLevel = 3;
    cltool.SuppressStartupBanner = true;
    cltool.CompileAs = 0; 
    cltool.Detect64BitPortabilityProblems = false;
    cltool.DebugInformationFormat = 3;
    cltool.UndefinePreprocessorDefinitions= '_WIN32';

    if (islib)
    {
      var libtool = config.Tools('VCLibrarianTool');
      libtool.OutputFile = '$(OutDir)\\$(ProjectName).lib';
      libtool.SuppressStartupBanner = true;
      libtool.IgnoreAllDefaultLibraries = true;
    }
    else
    {
      var linktool = config.Tools('VCLinkerTool');
      linktool.AdditionalOptions = '/MACHINE:I386 /FIXED:NO';
      if (useclib)
        linktool.AdditionalDependencies = 'os.lib libc.lib $(NOINHERIT)';
      else if (iskrnl)
        linktool.AdditionalDependencies = 'krnl.lib $(NOINHERIT)';
      else
        linktool.AdditionalDependencies = 'os.lib $(NOINHERIT)';
      if (isdrv) linktool.OutputFile = '$(OutDir)\\$(ProjectName).sys';

      linktool.LinkIncremental = 1;
      linktool.SuppressStartupBanner = true;
      if (extproj)
        linktool.AdditionalLibraryDirectories = '..\\dbg\\lib;' + sdkpath + '\\dbg\\lib';
      else
        linktool.AdditionalLibraryDirectories = sdkpath + '\\dbg\\lib';
      linktool.IgnoreAllDefaultLibraries = true;
      linktool.GenerateDebugInformation = true;
      
      if (!extproj)
        linktool.ProgramDatabaseFile = '$(OutDir)\\$(ProjectName).pdb';
      else
        linktool.ProgramDatabaseFile = '..\\dbg\\symbols\\$(ProjectName).pdb';
        
      linktool.GenerateMapFile = false;
      linktool.SubSystem = 1;
      if (isdll)
      {
      	linktool.ImportLibrary = '..\\dbg\\lib\\$(ProjectName).lib'
        if (wizard.FindSymbol('APP_TYPE_USERDLL'))
          linktool.EntryPointSymbol = 'DllMain';
        else
          linktool.EntryPointSymbol = 'start';
      }
    }

    config = proj.Object.Configurations('Release');

    if (!extproj)
    {
      config.IntermediateDirectory = 'release';
      config.OutputDirectory = 'release';
    }
    else
    {
      config.IntermediateDirectory = '..\\obj\\' + projname;
      if (islib)
        config.OutputDirectory = '..\\lib';
      else
        config.OutputDirectory = '..\\bin';
    }

    if (isdll) 
      config.ConfigurationType = 2;
    else if (islib)
      config.ConfigurationType = 4;

    var cltool = config.Tools('VCCLCompilerTool');
    cltool.Optimization = 2;
    cltool.GlobalOptimizations = true;
    cltool.InlineFunctionExpansion = 1;
    cltool.EnableIntrinsicFunctions = true;
    cltool.FavorSizeOrSpeed = 1;
    cltool.OmitFramePointers = true;
    //cltool.OptimizeForProcessor = 2;
    cltool.AdditionalIncludeDirectories = sdkpath + '\\src\\include';
    if (isdll || islib)
    {
      if (iskrnl)
        cltool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB;KERNEL;NDEBUG;SANOS';
      else
        cltool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB;NDEBUG;SANOS';
    }
    else
      cltool.PreprocessorDefinitions = prjname.toUpperCase() + ';NDEBUG;SANOS';

    if (defines != '')
    {
      cltool.PreprocessorDefinitions = cltool.PreprocessorDefinitions + ';' + defines;
    }

    cltool.IgnoreStandardIncludePath = true;
    cltool.StringPooling = true;
    cltool.ExceptionHandling = false;
    cltool.RuntimeLibrary = 0;
    cltool.BufferSecurityCheck = false;
    cltool.EnableFunctionLevelLinking = true;
    cltool.UsePrecompiledHeader = 2;
    cltool.PrecompiledHeaderThrough = '';
    cltool.PrecompiledHeaderFile = '$(IntDir)\\' + prjname + '.pch';
    cltool.WarningLevel = 3;
    cltool.SuppressStartupBanner = true;
    cltool.CompileAs = 0; 
    cltool.UndefineAllPreprocessorDefinitions = false;
    cltool.Detect64BitPortabilityProblems = false;
    cltool.UndefinePreprocessorDefinitions= '_WIN32';

    if (islib)
    {
      var libtool = config.Tools('VCLibrarianTool');
      libtool.OutputFile = '$(OutDir)\\$(ProjectName).lib';
      libtool.SuppressStartupBanner = true;
      libtool.IgnoreAllDefaultLibraries = true;
    }
    else
    {
      var linktool = config.Tools('VCLinkerTool');
      linktool.AdditionalOptions = '/MACHINE:I386 /FIXED:NO';
      if (useclib)
        linktool.AdditionalDependencies = 'os.lib libc.lib $(NOINHERIT)';
      else if (iskrnl)
        linktool.AdditionalDependencies = 'krnl.lib $(NOINHERIT)';
      else
        linktool.AdditionalDependencies = 'os.lib $(NOINHERIT)';
      if (isdrv) linktool.OutputFile = '$(OutDir)\\$(ProjectName).sys';
      linktool.LinkIncremental = 1;
      linktool.SuppressStartupBanner = true;
      if (extproj)
        linktool.AdditionalLibraryDirectories = '..\\lib;' + sdkpath + '\\lib';
      else
        linktool.AdditionalLibraryDirectories = sdkpath + '\\lib';
      linktool.IgnoreAllDefaultLibraries = true;
      linktool.GenerateDebugInformation = false;
      linktool.GenerateMapFile = false;
      linktool.SubSystem = 1;
      if (isdll)
      {
      	linktool.ImportLibrary = '..\\lib\\$(ProjectName).lib'
        if (wizard.FindSymbol('APP_TYPE_USERDLL'))
          linktool.EntryPointSymbol = 'DllMain';
        else
          linktool.EntryPointSymbol = 'start';
      }
    }
  }
  catch (e)
  {
    throw e;
  }
}

function CreateBootDiskProject(projpath, projname)
{
  var extproj = wizard.FindSymbol('EXT_PROJ');
  var sdkpath = wizard.FindSymbol('SANOS_SDKPATH');
  
  var builddir = projpath;
  if (extproj) builddir = builddir + '\\build';
    
  var projtemplatepath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
  var target = wizard.FindSymbol('TARGET');
  var proj = target.AddFromTemplate(projtemplatepath + '\\default.vcproj', builddir, 'bootdisk.vcproj');

  var config = proj.Object.Configurations('Debug');
  config.ConfigurationType = 10;
  if (extproj)
  {
    config.OutputDirectory = '..\\dbg';
    config.IntermediateDirectory = '..\\dbg\\bin';
  }
  else
  {
    config.OutputDirectory = 'debug';
    config.IntermediateDirectory = 'debug';
  }
  
  config = proj.Object.Configurations('Release');
  config.ConfigurationType = 10;
  if (extproj)
  {
    config.OutputDirectory = '..';
    config.IntermediateDirectory = '..\\bin';
  }
  else
  {
    config.OutputDirectory = 'release';
    config.IntermediateDirectory = 'release';
  }

  var inffile = CreateCustomInfFile('bootdsktmpl');

  if (extproj)
    AddFilesToCustomProj(proj, projpath + '\\build', inffile);
  else
    AddFilesToCustomProj(proj, projpath, inffile);

  inffile.Delete();

  var descr = 'Make floppy disk boot image';
  var cmdline = sdkpath + '\\tools\\mkdfs -d $(OutDir)\\' + projname + '.flp -b ' + sdkpath + '\\bin\\boot -l ' + sdkpath + '\\bin\\osldr.dll -k ' + sdkpath + '\\bin\\krnl.dll -c 1440 -i -f -F bootdisk.lst';
  var deps = '$(IntDir)\\' + projname + '.exe;' + builddir + '\\os.ini;' + builddir + '\\krnl.ini';
  var output = '$(OutDir)\\' + projname + '.flp';

  var bootlst = proj.ProjectItems('bootdisk.lst').Object;

  var fcfg = bootlst.FileConfigurations('Debug|Win32');
  var customtool = fcfg.Tool;

  customtool.Description = descr;
  customtool.CommandLine = cmdline + ' -a';
  customtool.AdditionalDependencies = deps;
  customtool.Outputs = output;

  fcfg = bootlst.FileConfigurations('Release|Win32');
  customtool = fcfg.Tool;

  customtool.Description = descr;
  customtool.CommandLine = cmdline;
  customtool.AdditionalDependencies = deps;
  customtool.Outputs = output;
  
  return proj;
}

function PchSettings(proj)
{
  // TODO: specify pch settings
}

function DelFile(fso, wiztempfile)
{
  try
  {
    if (fso.FileExists(wiztempfile))
    {
      var tmpfile = fso.GetFile(wiztempfile);
      tmpfile.Delete();
    }
  }
  catch (e)
  {
    throw e;
  }
}

function CreateCustomInfFile(name)
{
  try
  {
    var fso = new ActiveXObject('Scripting.FileSystemObject');
    var tfolder = fso.GetSpecialFolder(2);
    var tempfolder = tfolder.Drive + '\\' + tfolder.Name;

    var wiztempfile = tempfolder + '\\' + fso.GetTempName();

    var templatepath = wizard.FindSymbol('TEMPLATES_PATH');
    var inffile = templatepath + '\\' + name + '.inf';
    wizard.RenderTemplate(inffile, wiztempfile);

    return fso.GetFile(wiztempfile);
  }
  catch (e)
  {
    throw e;
  }
}

function GetTargetName(name)
{
  try
  {
    var prjname = wizard.FindSymbol('PROJECT_NAME');
    var target = name;

    if (name == 'main.c') target = prjname + '.c';

    return target; 
  }
  catch (e)
  {
    throw e;
  }
}

function AddFilesToCustomProj(proj, destpath, inffile)
{
  try
  {
    var templatepath = wizard.FindSymbol('TEMPLATES_PATH');

    var tpl = '';
    var name = '';

    var strm = inffile.OpenAsTextStream(1, -2);
    while (!strm.AtEndOfStream)
    {
      tpl = strm.ReadLine();
      if (tpl != '')
      {
        name = tpl;
        var target = GetTargetName(name);
        var template = templatepath + '\\' + tpl;
        var file = destpath + '\\' + target;

        var copyonly = false;
        var ext = name.substr(name.lastIndexOf('.'));
        if (ext == '.bmp' || ext == '.ico' || ext == '.gif' || ext == '.rtf' || ext == '.css') copyonly = true;
        wizard.RenderTemplate(template, file, copyonly);
        proj.Object.AddFile(file);
      }
    }
    strm.Close();
  }
  catch (e)
  {
    throw e;
  }
}
