#include "VR_PracticeWindowWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UVR_PracticeWindowWidget::SetPracticeProgress(float SprayProgress, float WipeProgress)
{
	SprayProgress = FMath::Clamp(SprayProgress, 0.f, 1.f);
	WipeProgress = FMath::Clamp(WipeProgress, 0.f, 1.f);

	if (ProgressBar_Spray)
	{
		ProgressBar_Spray->SetPercent(SprayProgress);
	}

	if (ProgressBar_Wipe)
	{
		ProgressBar_Wipe->SetPercent(WipeProgress);
	}

	if (Text_SprayPercent)
	{
		Text_SprayPercent->SetText(
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(SprayProgress * 100.f)))
		);
	}

	if (Text_WipePercent)
	{
		Text_WipePercent->SetText(
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(WipeProgress * 100.f)))
		);
	}
}