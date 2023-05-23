// Fill out your copyright notice in the Description page of Project Settings.


#include "InVideoWidget.h"
#include "Async/Async.h"
#include "Rendering/Texture2DResource.h"

void UInVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
void UInVideoWidget::NativeDestruct()
{
	StopPlay();
	Super::NativeDestruct();
}

bool UInVideoWidget::Init()
{
	return true;
}
uint32 UInVideoWidget::Run()
{
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget open Enter"));
	if (nullptr == m_WrapOpenCv)
	{
		m_WrapOpenCv = new WrapOpenCv();
	}
	if (false == m_WrapOpenCv->m_Stream.open(TCHAR_TO_UTF8(*m_VideoURL)))
	{
		UE_LOG(LogTemp, Error, TEXT("UInVideoWidget open url=%s"), *m_VideoURL);
		NotifyFailed();
		return -1;
	}
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget open END"));

	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget Run Enter"));
	while (false == m_Stopping)
	{
		if (false == m_WrapOpenCv->m_Stream.isOpened())
		{
			UE_LOG(LogTemp, Error, TEXT("UInVideoWidget Run isOpened"));
			NotifyFailed();
			return -1;
		}
		if (false == m_RealMode)
		{
			auto DataNow = FDateTime::Now();
			if ((DataNow - m_LastReadTime).GetTotalMilliseconds() < m_UpdateTime)
			{
				FPlatformProcess::Sleep(m_SleepSecond);
				continue;
			}
			m_LastReadTime = FDateTime::Now();
		}
		else
		{
			FPlatformProcess::Sleep(m_SleepSecond);
		}
		if (true == m_WrapOpenCv->m_Stream.read(m_WrapOpenCv->m_Frame))
		{
			NotifyFirstFrame();
			UpdateTexture();
		}
		
	}
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget Run END"));
	return 0;
}
void UInVideoWidget::Exit()
{

}
void UInVideoWidget::Stop()
{

}

void UInVideoWidget::StartPlay(const FString VideoURL, FDelegatePlayFailed Failed, FDelegateFirstFrame FirstFrame,const bool RealMode , const int Fps)
{
	StopPlay();
	m_Stopping = false;
	m_VideoURL = VideoURL;
	m_RealMode = RealMode;
	m_Fps = Fps;
	m_UpdateTime = 1000 / m_Fps;
	m_Failed = Failed;
	m_FirstFrame = FirstFrame;
	m_BFirstFrame = false;
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget StartPlay Enter"));
	m_Thread = FRunnableThread::Create(this, TEXT("Video Thread"));
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget StartPlay END"));
}
void UInVideoWidget::StopPlay()
{
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget StopPlay Enter"));
	m_Stopping = true;
	if (nullptr != m_Thread)
	{
		m_Thread->Kill();
		delete m_Thread;
		m_Thread = nullptr;
	}
	if (nullptr != m_WrapOpenCv)
	{
		if (m_WrapOpenCv->m_Stream.isOpened())
		{
			m_WrapOpenCv->m_Stream.release();
		}
		delete m_WrapOpenCv;
		m_WrapOpenCv = nullptr;
	}
	
	if (nullptr != m_VideoUpdateTextureRegion)
	{
		delete m_VideoUpdateTextureRegion;
		m_VideoUpdateTextureRegion = nullptr;
	}
	m_VideoSize = FVector2D(0, 0);
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget StopPlay END"));
}
void UInVideoWidget::NotifyFailed()
{
	FDelegatePlayFailed Failed = m_Failed;
	AsyncTask(ENamedThreads::GameThread, [Failed]()
		{
			if (Failed.IsBound())
				Failed.Execute();
		});
}
void UInVideoWidget::NotifyFirstFrame()
{
	if (m_BFirstFrame)
	{
		return;
	}
	m_BFirstFrame = true;
	FDelegateFirstFrame FirstFrame = m_FirstFrame;
	AsyncTask(ENamedThreads::GameThread, [FirstFrame]()
		{
			if (FirstFrame.IsBound())
				FirstFrame.Execute();
		});
}
void UInVideoWidget::UpdateTexture()
{
	if (nullptr == VideoTexture || m_VideoSize.X != m_WrapOpenCv->m_Frame.cols || m_VideoSize.Y != m_WrapOpenCv->m_Frame.rows)
	{
		UE_LOG(LogTemp, Log, TEXT("UInVideoWidget UpdateTexture First"));

		m_VideoSize = FVector2D(m_WrapOpenCv->m_Frame.cols, m_WrapOpenCv->m_Frame.rows);
		FEvent* SyncEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);
		AsyncTask(ENamedThreads::GameThread, [this, SyncEvent]()
		{
			VideoTexture = UTexture2D::CreateTransient(m_VideoSize.X, m_VideoSize.Y);
			if (VideoTexture)
			{
				VideoTexture->UpdateResource();
			}
			m_Texture2DResource = (FTexture2DResource*)VideoTexture->GetResource();
			UE_LOG(LogTemp, Log, TEXT("UInVideoWidget UpdateTexture CreateTransient"));
			SyncEvent->Trigger();
		});
		SyncEvent->Wait();
		FGenericPlatformProcess::ReturnSynchEventToPool(SyncEvent);

		UE_LOG(LogTemp, Log, TEXT("UInVideoWidget UpdateTexture CreateTransient End"));

		if (nullptr != m_VideoUpdateTextureRegion)
		{
			delete m_VideoUpdateTextureRegion;
			m_VideoUpdateTextureRegion = nullptr;
		}

		m_VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, m_VideoSize.X, m_VideoSize.Y);

		Data.Init(FColor(0, 0, 0, 255), m_VideoSize.X * m_VideoSize.Y);
	}
	if (nullptr == VideoTexture)
	{
		return;
	}

	for (int y = 0; y < m_VideoSize.Y; y++)
	{
		for (int x = 0; x < m_VideoSize.X; x++)
		{
			int i = x + (y * m_VideoSize.X);
			Data[i].B = m_WrapOpenCv->m_Frame.data[i * 3 + 0];
			Data[i].G = m_WrapOpenCv->m_Frame.data[i * 3 + 1];
			Data[i].R = m_WrapOpenCv->m_Frame.data[i * 3 + 2];
		}
	}
	UpdateTextureRegions(VideoTexture,(int32)0, (uint32)1, m_VideoUpdateTextureRegion, (uint32)(4 * m_VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);
	
	if (nullptr == ImageVideo)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		ImageVideo->SetBrushFromTexture(VideoTexture);
	});

}
void UInVideoWidget::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (m_Texture2DResource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};
		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = m_Texture2DResource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = new FUpdateTextureRegion2D(*Regions);
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)([RegionData, bFreeData](FRHICommandListImmediate& RHICmdList) {

			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
					);
				}
			}
			if (bFreeData)
			{
				//FMemory::Free(RegionData->Regions);
				//FMemory::Free(RegionData->SrcData);
				
			}
			delete RegionData->Regions;
			delete RegionData;
			});
	}
}