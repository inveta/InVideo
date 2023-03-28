// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "InRecordGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class INVIDEO_API UInRecordGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
public:
	void StartRecord(const int Fps);
	void StopRecord();
	DECLARE_DELEGATE_ThreeParams(FFrameDelegate, TArray<FColor>, int32, int32);
	FFrameDelegate OnFrameData;

	virtual void Draw(FViewport* InViewport, FCanvas* SceneCanvas) override;
private:
	bool m_CanRecord = false;
	double  m_FpsInterval = 10.0;
	double m_LastTime;
};
