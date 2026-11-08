#include "VR_WorkCompleteWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
//[추가] 플레이어 코드 Include
#include "VRPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UVR_WorkCompleteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Exit)
	{
		//Button_Exit->OnClicked.AddDynamic(this, &UVR_WorkCompleteWidget::OnExitClicked);
		Button_Exit->OnPressed.AddDynamic(this, &UVR_WorkCompleteWidget::OnExitClicked);
		Button_Exit->OnClicked.AddDynamic(this, &UVR_WorkCompleteWidget::OnExitClicked);
	}
}

void UVR_WorkCompleteWidget::UpdateInfo(float Money, int32 ElapsedSeconds)
{
	if (Text_Money)
	{
		Text_Money->SetText(
			FText::FromString(FString::Printf(TEXT("%.0f"), Money))
		);
	}

	if (Text_Time)
	{
		int32 Minutes = ElapsedSeconds / 60;
		int32 Seconds = ElapsedSeconds % 60;

		Text_Time->SetText(
			FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds))
		);
	}

	if (Text_Grade)
	{
		const FString Grade = GetGradeFromMoney(Money);

		Text_Grade->SetText(
			FText::FromString(Grade)
		);
	}
}

FString UVR_WorkCompleteWidget::GetGradeFromMoney(float Money) const
{
	if (Money >= 700000.f)
	{
		return TEXT("A");
	}
	else if (Money >= 650000.f)
	{
		return TEXT("B");
	}
	else if (Money >= 600000.f)
	{
		return TEXT("C");
	}

	return TEXT("F");
}

//[추가/변경] RPC 호출로 모든 플레이어를 타이틀 화면으로 복귀하도록 변경
void UVR_WorkCompleteWidget::OnExitClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	AVRPlayerCharacter* PlayerCharacter = Cast<AVRPlayerCharacter>(PC->GetPawn());

	if (!PlayerCharacter) return;

	PlayerCharacter->RequestReturnAllPlayersToTitle();
}