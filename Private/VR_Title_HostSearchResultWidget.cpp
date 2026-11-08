#include "VR_Title_HostSearchResultWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"


void UVR_Title_HostSearchResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_ResultHost) {
		Button_ResultHost->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	}
}

void UVR_Title_HostSearchResultWidget::NativeDestruct()
{
	if (Button_ResultHost) {
		Button_ResultHost->OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
	}

	Super::Destruct();
}

void UVR_Title_HostSearchResultWidget::SetupHostEntry(const FVR_HostSessionInfo& InHostInfo)
{
	SearchIndex = InHostInfo.SearchIndex;

	if (Text_HostInfo) {
		const FString InfoText = FString::Printf(TEXT("%s"), *InHostInfo.HostName);
		Text_HostInfo->SetText(FText::FromString(InfoText));
	}
}

void UVR_Title_HostSearchResultWidget::HandleClicked()
{
	OnSearchedHostClicked.Broadcast(SearchIndex);
}