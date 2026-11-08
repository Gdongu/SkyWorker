// Fill out your copyright notice in the Description page of Project Settings.

#include "SkyGameModeBase.h"
#include "SkyGameState.h"
#include "WindManager.h"
#include "VRPlayerState.h"
#include "VR_Dirt.h"
#include "VR_WaterResidue.h"
#include "Kismet/GameplayStatics.h"
#include "VRPlayerCharacter.h"
#include "Gondola.h"

ASkyGameModeBase::ASkyGameModeBase()
{
	PlayerStateClass = AVRPlayerState::StaticClass();
	GameStateClass = ASkyGameState::StaticClass();
}

void ASkyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (WindManagerClass)
	{
		AWindManager* WM = GetWorld()->SpawnActor<AWindManager>(WindManagerClass);
		if (WM)
		{
			WM->SetReplicates(true);
			WM->bAlwaysRelevant = true;
		}
	}

	// 1초마다 경과 시간 갱신 (타이머로 반복 호출)
	GetWorldTimerManager().SetTimer(ElapsedTimerHandle, this, &ASkyGameModeBase::IncrementElapsedTime, 1.f, true);

	TArray<AActor*> TempDirtActors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVR_Dirt::StaticClass(), CachedDirtActors);
	TotalDirtCount = CachedDirtActors.Num();
}

void ASkyGameModeBase::IncrementElapsedTime()
{
	ASkyGameState* GS = GetGameState<ASkyGameState>();
	if (GS)
	{
		GS->ElapsedSeconds++;

		if (GS->ElapsedSeconds % 300 == 0)
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (AVRPlayerState* VRS = Cast<AVRPlayerState>(PS))
					VRS->ApplyPenalty(50000.f);
				UE_LOG(LogTemp, Log, TEXT("Elapsed: %s"), *GS->GetFormattedTime().ToString());
			}
		}

		// 분사/닦기 진행도 계산
		TArray<AActor*> AliveDirtActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVR_Dirt::StaticClass(), AliveDirtActors);

		int32 CleanedDirtCount = TotalDirtCount - AliveDirtActors.Num();
		for (AActor* A : AliveDirtActors)
		{
			if (AVR_Dirt* Dirt = Cast<AVR_Dirt>(A))
			{
				if (Dirt->IsCleaned())
					CleanedDirtCount++;
			}
		}

		const int32 ClampedWipeCount = FMath::Clamp(WipeWaterCount, 0, TotalDirtCount);

		GS->SprayProgress = TotalDirtCount > 0
			? static_cast<float>(CleanedDirtCount) / static_cast<float>(TotalDirtCount)
			: 0.f;

		GS->WipeProgress = TotalDirtCount > 0
			? static_cast<float>(ClampedWipeCount) / static_cast<float>(TotalDirtCount)
			: 0.f;

		// 작업 완료 조건 개수 기준
		if (!GS->bWorkComplete &&
			TotalDirtCount > 0 &&
			CleanedDirtCount >= TotalDirtCount &&
			ClampedWipeCount >= TotalDirtCount)
		{
			GS->bWorkComplete = true;
			GetWorldTimerManager().ClearTimer(ElapsedTimerHandle);

			float DisplayMoney = 0.f;

			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (AVRPlayerState* VRS = Cast<AVRPlayerState>(PS))
				{
					DisplayMoney = VRS->Money;
					break;
				}
			}

			TArray<AActor*> Gondolas;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGondola::StaticClass(), Gondolas);

			for (AActor* Actor : Gondolas)
			{
				AGondola* Gondola = Cast<AGondola>(Actor);
				if (Gondola)
				{
					Gondola->MulticastShowWorkCompleteWidget(DisplayMoney, GS->ElapsedSeconds);
				}
			}
		}
	}
}


//[추가] 모든 플레이어 타이틀 화면 복귀
void ASkyGameModeBase::ReturnAllPlayersToTitle()
{
	if (bReturningToTitle) return;
	bReturningToTitle = true;

	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator IT = World->GetPlayerControllerIterator(); IT; ++IT) {
		APlayerController* PC = IT->Get();
		if (!PC) continue;

		if (PC->IsLocalController()) continue;

		AVRPlayerCharacter* VRCharacter = Cast<AVRPlayerCharacter>(PC->GetPawn());

		if (VRCharacter) VRCharacter->ClientReturnToTitle();
		else PC->ClientTravel(TEXT("/Game/VRTemplate/Maps/VRTemplateMap"), TRAVEL_Absolute);
	}

	FTimerHandle ReturnHostTimerHandle;

	World->GetTimerManager().SetTimer(ReturnHostTimerHandle, this, &ASkyGameModeBase::ReturnHostToTitle, 0.2f, false);
}

void ASkyGameModeBase::ReturnHostToTitle()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC) {
		AVRPlayerCharacter* HostCharacter = Cast<AVRPlayerCharacter>(PC->GetPawn());

		if (HostCharacter) {
			HostCharacter->LocalReturnToTitle();
			return;
		}
	}

	UGameInstance* GI = GetGameInstance();

	if (GI) GI->ReturnToMainMenu();

	UGameplayStatics::OpenLevel(this, FName("VRTemplateMap"));
}


