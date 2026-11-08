#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_SessionSubsystem.h"
#include "VR_Title_HostSearchResultWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchedHostClicked, int32, SearchIndex);

UCLASS()
class VR_PROJECT_API UVR_Title_HostSearchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupHostEntry(const FVR_HostSessionInfo& InHostInfo);

	UPROPERTY(BlueprintAssignable)
	FOnSearchedHostClicked OnSearchedHostClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ResultHost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_HostInfo;

	int32 SearchIndex = INDEX_NONE;
};
