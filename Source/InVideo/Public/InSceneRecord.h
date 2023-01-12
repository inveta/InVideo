// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "PostOpenCVHeaders.h"

#include "InSceneRecord.generated.h"

UCLASS()
class INVIDEO_API AInSceneRecord : public AActor, public FRunnable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInSceneRecord();

	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable, Category = "InVideo")
	void StartRecord(const FString FilePath,const int Fps = 25);

	UFUNCTION(BlueprintCallable, Category = "InVideo")
	void StoptRecord();

	void OnRequestFrame();
	void HandleFrameData(TArray<FColor> Bitmap, int32 x, int32 y);
public:
	bool Init() override;
	uint32 Run() override;
	void Stop() override;
	void Exit() override;

private:
	bool m_IsRecording = false;
	FString m_FilePath;
	int m_Fps = 0;
	char* m_ImageBuf = nullptr;
	int32 m_ImageX = 0;
	int32 m_ImageY = 0;
	
	class WrapOpenCv
	{
	public:
		cv::VideoWriter m_VideoWriter;
		TQueue<TArray<FColor>> m_ImageQueue;
	};
	WrapOpenCv* m_WrapOpenCv = nullptr;

	FRunnableThread* m_Thread = nullptr;
	TAtomic<bool> m_Stopping = false;

	
};
