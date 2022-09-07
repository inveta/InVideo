// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HAL/Runnable.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "PostOpenCVHeaders.h"

#include "InVideoWidget.generated.h"


/**
 * 
 */
UCLASS()
class INVIDEO_API UInVideoWidget : public UUserWidget ,public FRunnable
{
	GENERATED_BODY()
	
public:
	DECLARE_DYNAMIC_DELEGATE(FDelegatePlaySucceeded);
	DECLARE_DYNAMIC_DELEGATE(FDelegatePlayFailed);

	UFUNCTION(BlueprintCallable, Category = "InVideo")
    void StartPlay(const FString VideoURL, FDelegatePlayFailed Failed,
		const bool RealMode = true,const int Fps = 25);

	UFUNCTION(BlueprintCallable, Category = "InVideo")
	void StopPlay();

	UPROPERTY(BlueprintReadOnly, Category = "InVideo")
	UTexture2D* VideoTexture = nullptr;

	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget),Category = "InVideo")
	UImage* ImageVideo;
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	bool Init() override; 
	uint32 Run() override; 
	void Stop() override;
	void Exit() override;
private:
	void UpdateTexture();
	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
	void NotifyFailed();
private:
	FRunnableThread* m_Thread = nullptr;
	TAtomic<bool> m_Stopping = false;
	FString m_VideoURL;
	float m_UpdateTime = 20;
	int m_Fps = 25;
	bool m_RealMode = true;
	float m_SleepSecond = 1 / 50;
	FDateTime m_LastReadTime = FDateTime::Now();

	FDelegatePlayFailed m_Failed;

	class WrapOpenCv
	{
	public:
		cv::VideoCapture m_Stream;
		cv::Mat m_Frame;
	};
	WrapOpenCv *m_WrapOpenCv = nullptr;

	FVector2D m_VideoSize = FVector2D(0, 0);
	FUpdateTextureRegion2D* m_VideoUpdateTextureRegion = nullptr;
	FTexture2DResource* m_Texture2DResource = nullptr;
	TArray64<FColor> Data;
};

