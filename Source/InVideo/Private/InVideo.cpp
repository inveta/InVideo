// Copyright Epic Games, Inc. All Rights Reserved.

#include "InVideo.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"



#define LOCTEXT_NAMESPACE "FInVideoModule"

void FInVideoModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FInVideoModule StartupModule"));

	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("InVideo"))->GetBaseDir();
	const FString OpenCvBinPath = PluginDir / TEXT(PREPROCESSOR_TO_STRING(OPENCV_PLATFORM_PATH));
	const FString DLLPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_DLL_NAME));
	const FString DLLFFMPEGPath = OpenCvBinPath / TEXT(PREPROCESSOR_TO_STRING(OPENCV_DLL_FFMPEG));

	OpenCvDllHandle = FPlatformProcess::GetDllHandle(*DLLPath);
	OpenCvFfmpegDllHandle = FPlatformProcess::GetDllHandle(*DLLFFMPEGPath);

	if (nullptr == OpenCvDllHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("GetDllHandle DLLPath=%s"), *DLLPath);
	}
	if (nullptr == OpenCvFfmpegDllHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("GetDllHandle DLLFFMPEGPath=%s"), *DLLFFMPEGPath);
	}
}

void FInVideoModule::ShutdownModule()
{
	if (OpenCvDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvDllHandle);
		OpenCvDllHandle = nullptr;
	}
	if (OpenCvFfmpegDllHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenCvFfmpegDllHandle);
		OpenCvFfmpegDllHandle = nullptr;
	}
	UE_LOG(LogTemp, Log, TEXT("FInVideoModule ShutdownModule"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInVideoModule, InVideo)
