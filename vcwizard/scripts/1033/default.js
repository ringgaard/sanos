
function OnFinish(selProj, selObj)
{
  try
  {
    var strProjectPath = wizard.FindSymbol('PROJECT_PATH');
    var strProjectName = wizard.FindSymbol('PROJECT_NAME');

    selProj = CreateCustomProject(strProjectName, strProjectPath);
    AddConfig(selProj, strProjectName);

    var InfFile = CreateCustomInfFile();
    AddFilesToCustomProj(selProj, strProjectName, strProjectPath, InfFile);
    //PchSettings(selProj);
    InfFile.Delete();

    //DTE.Solution.SolutionBuild.SolutionConfigurations.Item('Debug').Delete();
    //DTE.Solution.SolutionBuild.SolutionConfigurations.Item('Release').Delete();

    selProj.Object.Save();
  }
  catch(e)
  {
    if (e.description.length != 0) SetErrorInfo(e);
    return e.number
  }
}

function CreateCustomProject(strProjectName, strProjectPath)
{
  try
  {
    var strProjTemplatePath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
    var strProjTemplate = '';
    strProjTemplate = strProjTemplatePath + '\\default.vcproj';

    var Solution = dte.Solution;
    var strSolutionName = "";
    if (wizard.FindSymbol("CLOSE_SOLUTION"))
    {
      Solution.Close();
      strSolutionName = wizard.FindSymbol("VS_SOLUTION_NAME");
      if (strSolutionName.length)
      {
        var strSolutionPath = strProjectPath.substr(0, strProjectPath.length - strProjectName.length);
        Solution.Create(strSolutionPath, strSolutionName);
      }
    }

    var strProjectNameWithExt = '';
    strProjectNameWithExt = strProjectName + '.vcproj';

    var oTarget = wizard.FindSymbol("TARGET");
    var prj;
    if (wizard.FindSymbol("WIZARD_TYPE") == vsWizardAddSubProject)  // vsWizardAddSubProject
    {
      var prjItem = oTarget.AddFromTemplate(strProjTemplate, strProjectNameWithExt);
      prj = prjItem.SubProject;
    }
    else
    {
      prj = oTarget.AddFromTemplate(strProjTemplate, strProjectPath, strProjectNameWithExt);
    }
    return prj;
  }
  catch(e)
  {
    throw e;
  }
}

function AddConfig(proj, strProjectName)
{
  try
  {
    var prjname = wizard.FindSymbol('PROJECT_NAME');
    var sdkpath = wizard.FindSymbol('SANOS_SDKPATH');
    var isdll = !wizard.FindSymbol('APP_TYPE_USEREXE');
    var isdrv = wizard.FindSymbol('APP_TYPE_KRNLDRV');
    var useclib = wizard.FindSymbol('USE_CLIB');
    
    //proj.Object.RemoveConfiguration(proj.Object.Configurations('Release'));
    //proj.Object.RemoveConfiguration(proj.Object.Configurations('Debug'));

    //proj.Object.AddConfiguration('Sanos');
    //proj.Object.AddConfiguration('SanosDebug');
    
    //var config = proj.Object.Configurations('SanosDebug');
    var config = proj.Object.Configurations('Debug');
    config.IntermediateDirectory = 'debug';
    config.OutputDirectory = 'debug';
    if (isdll) config.COnfigurationType = 2;

    var CLTool = config.Tools('VCCLCompilerTool');
    CLTool.Optimization = 0;
    CLTool.AdditionalIncludeDirectories = sdkpath + '\\src\\include';
    if (isdll)
      CLTool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB;DEBUG';
    else
      CLTool.PreprocessorDefinitions = prjname.toUpperCase() + ';DEBUG';
    CLTool.IgnoreStandardIncludePath = true;
    CLTool.ExceptionHandling = false;
    CLTool.RuntimeLibrary = 0;
    CLTool.BufferSecurityCheck = false;
    CLTool.UsePrecompiledHeader = 2;
    CLTool.PrecompiledHeaderFile = 'debug\\' + prjname + '.pch';
    CLTool.WarningLevel = 3;
    CLTool.SuppressStartupBanner = true;
    CLTool.CompileAs = 0; 
    CLTool.Detect64BitPortabilityProblems = false;
    
    var LinkTool = config.Tools('VCLinkerTool');
    LinkTool.AdditionalOptions = '/MACHINE:I386 /FIXED:NO';
    if (useclib)
      LinkTool.AdditionalDependencies = 'os.lib libc.lib $(NOINHERIT)';
    else
      LinkTool.AdditionalDependencies = 'os.lib $(NOINHERIT)';
    if (isdrv) LinkTool.OutputFile = '$(OutDir)/$(ProjectName).sys';
    LinkTool.LinkIncremental = 1;
    LinkTool.SuppressStartupBanner = true;
    LinkTool.AdditionalLibraryDirectories = sdkpath + "\\dbg\\lib";
    LinkTool.IgnoreAllDefaultLibraries = true;
    LinkTool.GenerateDebugInformation = true;
    LinkTool.GenerateMapFile = false;
    LinkTool.SubSystem = 1;
    if (isdll)
    {
      if (wizard.FindSymbol('APP_TYPE_USERDLL'))
        LinkTool.EntryPointSymbol = 'DllMain';
      else
        LinkTool.EntryPointSymbol = 'start';
    }

    //config = proj.Object.Configurations('Sanos');
    config = proj.Object.Configurations('Release');

    config.IntermediateDirectory = 'release';
    config.OutputDirectory = 'release';
    if (isdll) config.ConfigurationType = 2;

    var CLTool = config.Tools('VCCLCompilerTool');
    CLTool.Optimization = 2;
    CLTool.GlobalOptimizations = true;
    CLTool.InlineFunctionExpansion = 1;
    CLTool.EnableIntrinsicFunctions = true;
    CLTool.FavorSizeOrSpeed = 1;
    CLTool.OmitFramePointers = true;
    //CLTool.OptimizeForProcessor = 2;
    CLTool.AdditionalIncludeDirectories = sdkpath + '\\src\\include';
    if (isdll)
      CLTool.PreprocessorDefinitions = prjname.toUpperCase() + '_LIB';
    else
      CLTool.PreprocessorDefinitions = prjname.toUpperCase();
    CLTool.IgnoreStandardIncludePath = true;
    CLTool.StringPooling = true;
    CLTool.ExceptionHandling = false;
    CLTool.RuntimeLibrary = 0;
    CLTool.EnableFunctionLevelLinking = true;
    CLTool.UsePrecompiledHeader = 2;
    CLTool.PrecompiledHeaderFile = 'release\\' + prjname + '.pch';
    CLTool.WarningLevel = 3;
    CLTool.SuppressStartupBanner = true;
    CLTool.CompileAs = 0; 
    CLTool.UndefineAllPreprocessorDefinitions = false;
    CLTool.Detect64BitPortabilityProblems = false;

    var LinkTool = config.Tools('VCLinkerTool');
    LinkTool.AdditionalOptions = '/MACHINE:I386 /FIXED:NO';
    if (useclib)
      LinkTool.AdditionalDependencies = 'os.lib libc.lib $(NOINHERIT)';
    else
      LinkTool.AdditionalDependencies = 'os.lib $(NOINHERIT)';
    if (isdrv) LinkTool.OutputFile = '$(OutDir)/$(ProjectName).sys';
    LinkTool.LinkIncremental = 1;
    LinkTool.SuppressStartupBanner = true;
    LinkTool.AdditionalLibraryDirectories = sdkpath + "\\lib";
    LinkTool.IgnoreAllDefaultLibraries = true;
    LinkTool.GenerateDebugInformation = true;
    LinkTool.GenerateMapFile = false;
    LinkTool.SubSystem = 1;
    if (isdll)
    {
      if (wizard.FindSymbol('APP_TYPE_USERDLL'))
        LinkTool.EntryPointSymbol = 'DllMain';
      else
        LinkTool.EntryPointSymbol = 'start';
    }
  }
  catch(e)
  {
    throw e;
  }
}

function PchSettings(proj)
{
  // TODO: specify pch settings
}

function DelFile(fso, strWizTempFile)
{
  try
  {
    if (fso.FileExists(strWizTempFile))
    {
      var tmpFile = fso.GetFile(strWizTempFile);
      tmpFile.Delete();
    }
  }
  catch(e)
  {
    throw e;
  }
}

function CreateCustomInfFile()
{
  try
  {
    var fso, TemplatesFolder, TemplateFiles, strTemplate;
    fso = new ActiveXObject('Scripting.FileSystemObject');

    var TemporaryFolder = 2;
    var tfolder = fso.GetSpecialFolder(TemporaryFolder);
    var strTempFolder = tfolder.Drive + '\\' + tfolder.Name;

    var strWizTempFile = strTempFolder + "\\" + fso.GetTempName();

    var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
    var strInfFile = strTemplatePath + '\\Templates.inf';
    wizard.RenderTemplate(strInfFile, strWizTempFile);

    var WizTempFile = fso.GetFile(strWizTempFile);
    return WizTempFile;
  }
  catch(e)
  {
    throw e;
  }
}

function GetTargetName(strName, strProjectName)
{
  try
  {
    var prjname = wizard.FindSymbol('PROJECT_NAME');
    var strTarget = strName;

    if (strName == 'main.c') strTarget = prjname + '.c';

    return strTarget; 
  }
  catch(e)
  {
    throw e;
  }
}

function AddFilesToCustomProj(proj, strProjectName, strProjectPath, InfFile)
{
  try
  {
    var projItems = proj.ProjectItems

    var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');

    var strTpl = '';
    var strName = '';

    var strTextStream = InfFile.OpenAsTextStream(1, -2);
    while (!strTextStream.AtEndOfStream)
    {
      strTpl = strTextStream.ReadLine();
      if (strTpl != '')
      {
        strName = strTpl;
        var strTarget = GetTargetName(strName, strProjectName);
        var strTemplate = strTemplatePath + '\\' + strTpl;
        var strFile = strProjectPath + '\\' + strTarget;

        var bCopyOnly = false;  //"true" will only copy the file from strTemplate to strTarget without rendering/adding to the project
        var strExt = strName.substr(strName.lastIndexOf("."));
        if (strExt==".bmp" || strExt==".ico" || strExt==".gif" || strExt==".rtf" || strExt==".css") bCopyOnly = true;
        wizard.RenderTemplate(strTemplate, strFile, bCopyOnly);
        proj.Object.AddFile(strFile);
      }
    }
    strTextStream.Close();
  }
  catch(e)
  {
    throw e;
  }
}
