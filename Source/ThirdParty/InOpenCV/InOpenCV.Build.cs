// Copyright Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using UnrealBuildTool;

public class InOpenCV : ModuleRules
{
  public InOpenCV(ReadOnlyTargetRules Target) : base(Target)
  {
    Type = ModuleType.External;

    string PlatformDir = Target.Platform.ToString();
    string IncPath = Path.Combine(ModuleDirectory, "include");
    string BinaryPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/ThirdParty", PlatformDir));

    if (Target.Platform == UnrealTargetPlatform.Win64)
    {
      PublicSystemIncludePaths.Add(IncPath);

      string LibPath = Path.Combine(ModuleDirectory, "lib", PlatformDir);
      string LibName = "opencv_world460";
      string LibFfmpeg = "opencv_videoio_ffmpeg460_64";

      PublicAdditionalLibraries.Add(Path.Combine(LibPath, LibName + ".lib"));
      string DLLName = LibName + ".dll";
      string DLLFfmpeg = LibFfmpeg + ".dll";
      PublicDelayLoadDLLs.Add(DLLName);
      PublicDelayLoadDLLs.Add(DLLFfmpeg);
      RuntimeDependencies.Add(Path.Combine(BinaryPath, DLLName));
      RuntimeDependencies.Add(Path.Combine(BinaryPath, DLLFfmpeg));

      PublicDefinitions.Add("OPENCV_PLATFORM_PATH=Binaries/ThirdParty/" + PlatformDir);
      PublicDefinitions.Add("OPENCV_DLL_NAME=" + DLLName);
      PublicDefinitions.Add("OPENCV_DLL_FFMPEG=" + DLLFfmpeg);
    }
    else // unsupported platform
    {

    }
  }
}
