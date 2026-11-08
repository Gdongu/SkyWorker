#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_WorkWatchWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class VR_PROJECT_API UVR_WorkWatchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Work Watch")
	void UpdateWorkInfo(float ProgressPercent, const FString& CurrentZone, float ElapsedSeconds); // 작업 진행 상황 업데이트 함수

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Work; // 작업 진행 상황을 시각적으로 나타내는 ProgressBar

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Progress; // 작업 진행 상황을 퍼센트로 나타내는 TextBlock

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Zone; // 현재 작업 구역을 나타내는 TextBlock

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Time; // 작업 시작 후 경과 시간을 나타내는 TextBlock
};