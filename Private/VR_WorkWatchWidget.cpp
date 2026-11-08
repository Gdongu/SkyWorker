#include "VR_WorkWatchWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UVR_WorkWatchWidget::UpdateWorkInfo(float ProgressPercent, const FString& CurrentZone, float ElapsedSeconds)
{
	UE_LOG(LogTemp, Warning, TEXT("[WatchWidget] Text_Time=%s Text_Zone=%s Bar=%s"),
		Text_Time ? TEXT("OK") : TEXT("NULL"),
		Text_Zone ? TEXT("OK") : TEXT("NULL"),
		ProgressBar_Work ? TEXT("OK") : TEXT("NULL"));

	const float ClampedProgress = FMath::Clamp(ProgressPercent, 0.f, 1.f);

	if (ProgressBar_Work)
	{
		ProgressBar_Work->SetPercent(ClampedProgress);
	}

	if (Text_Progress)
	{
		const int32 PercentInt = FMath::RoundToInt(ClampedProgress * 100.f);
		Text_Progress->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentInt)));
	}

	if (Text_Zone)
	{
		Text_Zone->SetText(FText::FromString(CurrentZone));
	}

	if (Text_Time)
	{
		const int32 TotalSeconds = FMath::FloorToInt(ElapsedSeconds); // 소수점 이하 버림
		// 분과 초로 변환
		const int32 Minutes = TotalSeconds / 60;
		const int32 Seconds = TotalSeconds % 60;
		Text_Time->SetText(
			FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)) // 00:00 형식으로 표시
		);
	}
}