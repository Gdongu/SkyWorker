#include "VR_Title_GameMode.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "VR_SessionSubsystem.h"

AVR_Title_GameMode::AVR_Title_GameMode() {
	bStartPlayersAsSpectators = false;
}

void AVR_Title_GameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);

	if (NewPlayer && NewPlayer->PlayerState && GameState) {
		int32 PlayerNumber = GameState->PlayerArray.Num();
		FString NewName = FString::Printf(TEXT("Player %d"), PlayerNumber);
		NewPlayer->PlayerState->SetPlayerName(NewName);
	}

	int32 CurrentPlayers = GameState ? GameState->PlayerArray.Num() : 0;
}

void AVR_Title_GameMode::Logout(AController* Existing)
{
	Super::Logout(Existing);

	int32 CurrentPlayers = GameState ? GameState->PlayerArray.Num() : 0;
}

void AVR_Title_GameMode::StartGameFromLobby()
{
	if (!HasAuthority()) return;
	if (!GameState) return;

	int32 CurrentPlayers = GameState->PlayerArray.Num();

	//플레이어 수가 총 인원수에 맞지 않으면 Return (차후 변형도 가능)
	if (CurrentPlayers < RequiredPlayerCount) return;
	//각 Session을 게임 시작 상태로 변경
	if (UGameInstance* GI = GetGameInstance()) {
		if (UVR_SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UVR_SessionSubsystem>()) {
			SessionSubsystem->MarkSessionStarted();
		}
	}

	//인게임 맵으로 이동
	FString TravelURL = GameMapPath + TEXT("?listen");
	GetWorld()->ServerTravel(TravelURL);
}


