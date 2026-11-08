#include "VR_GondolaCountWidget.h"
#include "Components/TextBlock.h"

void UVR_GondolaCountWidget::SetCountText(int32 CurrentCount, int32 RequiredCount)
{
	if (!Text_Count) return;
	if (bIsLoading) return;

	const FString CountString = FString::Printf(TEXT("%d/%d"), CurrentCount, RequiredCount); // 현재 탑승 인원 / 최대 탑승 인원 형식으로 문자열 생성
	Text_Count->SetText(FText::FromString(CountString)); // Text_Count에 생성한 문자열을 설정하여 UI에 표시
}

void UVR_GondolaCountWidget::SetLoadingText()
{
	if (!Text_Count) return;
	Text_Count->SetRenderScale(FVector2D(0.5f, 0.5f));
	Text_Count->SetText(FText::FromString("Start"));
}
