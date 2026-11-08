#include "VR_WindWarningWidget.h"
#include "Components/TextBlock.h"

void UVR_WindWarningWidget::SetCountdownNumber(int32 Count)
{
	if (!Text_Countdown) return;

	Text_Countdown->SetText(
		FText::FromString(FString::Printf(TEXT("%d"), Count))
	);
}