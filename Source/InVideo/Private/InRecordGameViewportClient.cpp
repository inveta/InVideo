// Fill out your copyright notice in the Description page of Project Settings.


#include "InRecordGameViewportClient.h"
#include "Slate/SceneViewport.h"


void UInRecordGameViewportClient::StartRecord(const int Fps)
{
	m_CanRecord = true;
}

void UInRecordGameViewportClient::StopRecord()
{
	m_CanRecord = false;
}
void UInRecordGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (false == m_CanRecord)
	{
		return;
	}
	m_TickRecord = true;
}

void UInRecordGameViewportClient::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	Super::Draw(InViewport, SceneCanvas);

	if (false == m_TickRecord)
	{
		return;
	}
	m_TickRecord = false;

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