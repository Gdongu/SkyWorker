#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VR_Title_GameMode.generated.h"

UCLASS()
class VR_PROJECT_API AVR_Title_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVR_Title_GameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Existing) override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartGameFromLobby();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	int32 RequiredPlayerCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	FString GameMapPath = TEXT("/Game/Levels/L_Tutorial");
};
