#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_GondolaCountWidget.generated.h"

class UTextBlock;

UCLASS()
class VR_PROJECT_API UVR_GondolaCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetCountText(int32 CurrentCount, int32 RequiredCount);

	UFUNCTION(BlueprintCallable)
	void SetLoadingText();

	UPROPERTY()
	bool bIsLoading = false;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Count; // 탑승 인원 수를 표시할 텍스트 블록
};