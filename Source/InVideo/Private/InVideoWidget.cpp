// Fill out your copyright notice in the Description page of Project Settings.


#include "InVideoWidget.h"
#include "Async/Async.h"
#include "Rendering/Texture2DResource.h"

void UInVideoWidget::NativeConstruct()
{
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget NativeConstruct"));
	Super::NativeConstruct();
}
void UInVideoWidget::NativeDestruct()
{
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget NativeDestruct"));
	StopPlay();
	Super::NativeDestruct();
}


void UInVideoWidget::StartPlay(const FString VideoURL, FDelegatePlayFailed Failed, FDelegateFirstFrame FirstFrame,const bool RealMode , const int Fps)
{
	StopPlay();
	m_VideoPlayPtr = MakeUnique<VideoPlay>();
	m_VideoPlayPtr->StartPlay(VideoURL, Failed, FirstFrame, RealMode, Fps,this);
}
void UInVideoWidget::StopPlay()
{
	if (m_VideoPlayPtr.Get() == nullptr)
	{
		return;
	}
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ptr = MoveTemp(m_VideoPlayPtr)]()
		{
		 ptr->StopPlay();
		});
}



void VideoPlay::StartPlay(const FString VideoURL, FDelegatePlayFailed Failed, FDelegateFirstFrame FirstFrame, const bool RealMode, const int Fps, UInVideoWidget* widget)
{
	StopPlay();
	m_widget = widget;
	m_Stopping = false;
	m_VideoURL = VideoURL;
	m_RealMode = RealMode;
	m_Fps = Fps;
	m_UpdateTime = 1000 / m_Fps;
	m_Failed = Failed;
	m_FirstFrame = FirstFrame;
	m_BFirstFrame = false;
	UE_LOG(LogTemp, Log, TEXT("VideoPlay StartPlay Enter"));
	m_Thread = FRunnableThread::Create(this, TEXT("Video Thread"));
	UE_LOG(LogTemp, Log, TEXT("VideoPlay StartPlay END"));
}
void VideoPlay::StopPlay()
{
	UE_LOG(LogTemp, Log, TEXT("VideoPlay StopPlay Enter"));
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
	AsyncTask(ENamedThreads::GameThread, [vt = VideoTexture]()
		{
			if (vt->IsValidLowLevel()) 
			{
				vt->RemoveFromRoot();
			}
		});
	m_VideoSize = FVector2D(0, 0);
	UE_LOG(LogTemp, Log, TEXT("UInVideoWidget StopPlay END"));
}
bool VideoPlay::Init()
{
	return true;
}
uint32 VideoPlay::Run()
{
	UE_LOG(LogTemp, Log, TEXT("VideoPlay open Enter"));
	if (nullptr == m_WrapOpenCv)
	{
		m_WrapOpenCv = new WrapOpenCv();
	}
	if (false == m_WrapOpenCv->m_Stream.open(TCHAR_TO_UTF8(*m_VideoURL)))
	{
		UE_LOG(LogTemp, Error, TEXT("VideoPlay open url=%s"), *m_VideoURL);
		NotifyFailed();
		return -1;
	}
	UE_LOG(LogTemp, Log, TEXT("VideoPlay open END"));

	UE_LOG(LogTemp, Log, TEXT("VideoPlay Run Enter"));
	while (false == m_Stopping)
	{
		if (false == m_WrapOpenCv->m_Stream.isOpened())
		{
			UE_LOG(LogTemp, Error, TEXT("VideoPlay Run isOpened"));
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
	UE_LOG(LogTemp, Log, TEXT("VideoPlay Run END"));
	return 0;
}
void VideoPlay::Exit()
{

}
void VideoPlay::Stop()
{

}
void VideoPlay::NotifyFailed()
{
	AsyncTask(ENamedThreads::GameThread, [Failed= m_Failed]()
		{
			if (Failed.IsBound())
				Failed.Execute();
		});
}
void VideoPlay::NotifyFirstFrame()
{
	if (m_BFirstFrame)
	{
		return;
	}
	m_BFirstFrame = true;
	AsyncTask(ENamedThreads::GameThread, [FirstFrame = m_FirstFrame]()
		{
			if (FirstFrame.IsBound())
				FirstFrame.Execute();
		});
}
void VideoPlay::UpdateTexture()
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
				VideoTexture->AddToRoot();
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
	UpdateTextureRegions(VideoTexture, (int32)0, (uint32)1, m_VideoUpdateTextureRegion, (uint32)(4 * m_VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);

	AsyncTask(ENamedThreads::GameThread, [vt = VideoTexture, widget = m_widget]()
		{
			if (false == widget->IsValidLowLevel())
			{
				return;
			}
			if (nullptr == widget->ImageVideo)
			{
				return;
			}
			if (false == vt->IsValidLowLevel())
			{
				return;
			}
			widget->ImageVideo->SetBrushFromTexture(vt);
		});

}
void VideoPlay::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
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
		RegionData->SrcData = new uint8[Regions->Width* Regions->Height*4];
		memcpy(RegionData->SrcData, SrcData, Regions->Width* Regions->Height * 4);

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
			delete[] RegionData->SrcData;
			delete RegionData;
			});
	}
}