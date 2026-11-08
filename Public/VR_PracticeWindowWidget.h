#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_PracticeWindowWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class VR_PROJECT_API UVR_PracticeWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Practice UI")
	void SetPracticeProgress(float SprayProgress, float WipeProgress);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Spray;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Wipe;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SprayPercent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WipePercent;
};