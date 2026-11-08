#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_BucketRefillWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class VR_PROJECT_API UVR_BucketRefillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bucket Refill")
	void UpdateRefillProgress(float CurrentWater, float MaxWater);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Water;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WaterPercent;
};