// Fill out your copyright notice in the Description page of Project Settings.


#include "InRecordGameViewportClient.h"
#include "Slate/SceneViewport.h"


void UInRecordGameViewportClient::StartRecord(const int Fps)
{
	m_CanRecord = true;
	m_FpsInterval = 1000.0 / Fps;
	m_LastTime = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
}

void UInRecordGameViewportClient::StopRecord()
{
	m_CanRecord = false;
}

void UInRecordGameViewportClient::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	Super::Draw(InViewport, SceneCanvas);

	if (false == m_CanRecord)
	{
		return;
	}

	auto nowTime = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
	if ((nowTime - m_LastTime) < m_FpsInterval)
	{
		return;
	}
	m_LastTime = nowTime;

	auto SceneViewport = GetGameViewport();

	TArray<FColor> Bitmap;
	int BitmapX = InViewport->GetSizeXY().X;
	int BitmapY = InViewport->GetSizeXY().Y;

	auto ReadRet = SceneViewport->ReadPixels(Bitmap, FReadSurfaceDataFlags());
	if (false == ReadRet)
	{
		return;
	}
	OnFrameData.ExecuteIfBound(Bitmap, BitmapX, BitmapY);
}