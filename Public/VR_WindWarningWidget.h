#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_WindWarningWidget.generated.h"

class UTextBlock;

UCLASS()
class VR_PROJECT_API UVR_WindWarningWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Wind Warning")
	void SetCountdownNumber(int32 Count);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Countdown;
};