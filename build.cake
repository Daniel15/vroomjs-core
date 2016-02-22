#l "common.cake"

//////////////////////////////////////////////////////////////////////
// ARGUMENTS
//////////////////////////////////////////////////////////////////////

var target = Argument("target", "Default");
var configuration = Argument("configuration", "Release");

//////////////////////////////////////////////////////////////////////
// PREPARATION
//////////////////////////////////////////////////////////////////////

var buildNumber=9999;
var baseDir=System.IO.Directory.GetCurrentDirectory();
var buildDir=System.IO.Path.Combine(baseDir, "build");
var distDir=System.IO.Path.Combine(baseDir, "dist");
var isRunningOnAppVeyor = AppVeyor.IsRunningOnAppVeyor;
if(isRunningOnAppVeyor)
    buildNumber = AppVeyor.Environment.Build.Number;
System.Environment.SetEnvironmentVariable("DNX_BUILD_VERSION", buildNumber.ToString(), System.EnvironmentVariableTarget.Process);

//////////////////////////////////////////////////////////////////////
// TASKS
//////////////////////////////////////////////////////////////////////

Task("EnsureDependencies")
    .Does(() =>
{
    EnsureTool("dnx", "--version");
    EnsureTool("dnu", "--version");
});

Task("Clean")
    .Does(() =>
{
    CleanDirectory(buildDir);
    CleanDirectory(distDir);
});

Task("Build")
    .Does(() =>
{
    ExecuteCommand("dnu restore \"src/VroomJs/project.json\"");
    ExecuteCommand(string.Format("dnu publish \"src/VroomJs/project.json\" --configuration \"{0}\" --no-source -o \"{1}\"", configuration, System.IO.Path.Combine(buildDir, "VroomJs")));
});

Task("Test")
    .WithCriteria(() => !isRunningOnAppVeyor)
    .Does(() =>
{
    // no tests
});

Task("Deploy")
    .Does(() =>
{
    if(!DirectoryExists(distDir))
        CreateDirectory(distDir);

    var destination =  System.IO.Path.Combine(distDir, "VroomJs");
    if(!DirectoryExists(destination))
        CreateDirectory(destination);
    CopyDirectory(System.IO.Path.Combine(buildDir, "VroomJs"), destination);
});

//////////////////////////////////////////////////////////////////////
// TASK TARGETS
//////////////////////////////////////////////////////////////////////

Task("Default")
    .IsDependentOn("EnsureDependencies")
    .IsDependentOn("Clean")
    .IsDependentOn("Build")
    .IsDependentOn("Test");

Task("CI")
    .IsDependentOn("EnsureDependencies")
    .IsDependentOn("Clean")
    .IsDependentOn("Build")
    .IsDependentOn("Test")
    .IsDependentOn("Deploy");

//////////////////////////////////////////////////////////////////////
// EXECUTION
//////////////////////////////////////////////////////////////////////

RunTarget(target);
