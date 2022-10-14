// Fill out your copyright notice in the Description page of Project Settings.


#include "InSceneRecord.h"
#include "InRecordGameViewportClient.h"

#include <string>

// Sets default values
AInSceneRecord::AInSceneRecord()
{

}

void AInSceneRecord::Destroyed()
{
	if (nullptr != m_ImageBuf)
	{
		delete[] m_ImageBuf;
		m_ImageBuf = nullptr;
	}
	if (nullptr != m_WrapOpenCv)
	{
		m_WrapOpenCv->m_VideoWriter.release();
		delete m_WrapOpenCv;
		m_WrapOpenCv = nullptr;
	}
	
}
void AInSceneRecord::StartRecord(const FString FilePath, const int Fps)
{
	UE_LOG(LogTemp, Log, TEXT("AInSceneRecord StartRecord FilePath=%s"),*FilePath);
	if (true == m_IsRecording)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord StartRecord IsRecording ture"));
		return;
	}
	m_IsRecording = true;
	m_FilePath = FilePath;
	m_Fps = Fps;
	if (false == FPaths::ValidatePath(m_FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord StartRecord ValidatePath m_FilePath=%s"), *m_FilePath);
		return;
	}
	FString FoldPath = FPaths::GetPath(m_FilePath);
	if (false == FPaths::DirectoryExists(FoldPath))
	{
		UE_LOG(LogTemp, Log, TEXT("AInSceneRecord StartRecord CreateDirectoryTree=%s"), *FoldPath);
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FoldPath);
	}
	auto world = GetWorld();
	if (nullptr == world)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord StartRecord GetWorld false"));
		return;
	}

	UInRecordGameViewportClient* ViewPortClient = Cast<UInRecordGameViewportClient>(
		world->GetGameViewport());
	if (nullptr == ViewPortClient)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord StartRecord UInRecordGameViewportClient nullptr"));
		return;
	}
	if (true == ViewPortClient->OnFrameData.IsBound())
	{
		ViewPortClient->OnFrameData.Unbind();
	}
	ViewPortClient->OnFrameData.BindUObject(this, &AInSceneRecord::HandleFrameData);

	if (nullptr != m_ImageBuf)
	{
		delete[] m_ImageBuf;
		m_ImageBuf = nullptr;
	}
	if (nullptr == m_WrapOpenCv)
	{
		m_WrapOpenCv = new WrapOpenCv();
	}

	GetWorld()->GetTimerManager().SetTimer(m_TimeHandle,this, &AInSceneRecord::OnRequestFrame,1.0f/ Fps, true, 0);
}

void AInSceneRecord::StoptRecord()
{
	UE_LOG(LogTemp, Log, TEXT("AInSceneRecord StoptRecord "));
	if (false == m_IsRecording)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord StoptRecord IsRecording false"));
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(m_TimeHandle);

	if (nullptr != m_ImageBuf)
	{
		delete[] m_ImageBuf;
		m_ImageBuf = nullptr;
	}
	m_WrapOpenCv->m_VideoWriter.release();
}

void AInSceneRecord::OnRequestFrame()
{
	auto world = GetWorld();
	if (nullptr == world)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord OnRequestFrame GetWorld false"));
		return;
	}

	UInRecordGameViewportClient* ViewPortClient = Cast<UInRecordGameViewportClient>(
		world->GetGameViewport());

	if (nullptr == ViewPortClient)
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord OnRequestFrame UInRecordGameViewportClient nullptr"));
		return;
	}
	ViewPortClient->RequestFrame();
}
void AInSceneRecord::HandleFrameData(TArray<FColor> Bitmap, int32 x, int32 y)
{
	int length = Bitmap.Num() * 3;
	if (nullptr != m_ImageBuf)
	{
		if (m_ImageX != x || m_ImageY != y)
		{
			UE_LOG(LogTemp, Error, TEXT("AInSceneRecord HandleFrameData m_ImageX=%d m_ImageY=%d x=%d y=%d"), m_ImageX, m_ImageY,x,y);
			return;
		}
	}
	if (nullptr == m_ImageBuf)
	{
		UE_LOG(LogTemp, Log, TEXT("AInSceneRecord HandleFrameData x=%d y=%d"), x,y);

		m_ImageBuf = new char[length];
		m_ImageX = x;
		m_ImageY = y;
		std::string cvFilePath(TCHAR_TO_UTF8(*m_FilePath));

		m_WrapOpenCv->m_VideoWriter.release();
		m_WrapOpenCv->m_VideoWriter.open(cvFilePath, cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), m_Fps, cv::Size(m_ImageX, m_ImageY));
	}

	int count = 0;
	for (int i = 0; i < Bitmap.Num(); i++)
	{
		m_ImageBuf[count] = Bitmap[i].B;
		m_ImageBuf[count + 1] = Bitmap[i].G;
		m_ImageBuf[count + 2] = Bitmap[i].R;
		count += 3;
	}
	
	if (false == m_WrapOpenCv->m_VideoWriter.isOpened())
	{
		UE_LOG(LogTemp, Error, TEXT("AInSceneRecord m_VideoWriter isOpened"));
		return;
	}
	cv::Mat img(m_ImageY,m_ImageX,CV_8UC3, (unsigned char*)m_ImageBuf);
	m_WrapOpenCv->m_VideoWriter.write(img);
}