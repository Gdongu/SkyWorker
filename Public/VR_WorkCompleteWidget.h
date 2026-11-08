#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_WorkCompleteWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class VR_PROJECT_API UVR_WorkCompleteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Work Complete")
	void UpdateInfo(float Money, int32 ElapsedSeconds);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Money;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Time;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Grade;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Exit;

	UFUNCTION()
	void OnExitClicked();

private:
	FString GetGradeFromMoney(float Money) const;
};