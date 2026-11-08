#include "VR_BucketRefillWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UVR_BucketRefillWidget::UpdateRefillProgress(float CurrentWater, float MaxWater)
{
	const float Percent = MaxWater > 0.f
		? FMath::Clamp(CurrentWater / MaxWater, 0.f, 1.f)
		: 0.f;

	if (ProgressBar_Water)
	{
		ProgressBar_Water->SetPercent(Percent);
	}

	if (Text_WaterPercent)
	{
		const int32 PercentInt = FMath::RoundToInt(Percent * 100.f);
		Text_WaterPercent->SetText(
			FText::FromString(FString::Printf(TEXT("%d%%"), PercentInt))
		);
	}
}