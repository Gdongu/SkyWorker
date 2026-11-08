#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VR_Title_PlayerController.generated.h"

class UVR_Title_LobbyWidget;

UCLASS()
class VR_PROJECT_API AVR_Title_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UVR_Title_LobbyWidget> VR_Title_LobbyWidget;

	UPROPERTY()
	TObjectPtr<UVR_Title_LobbyWidget> LobbyWidget;
};
