// Fill out your copyright notice in the Description page of Project Settings.


#include "WindManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "VRPlayerCharacter.h"

AWindManager::AWindManager()
{
    PrimaryActorTick.bCanEverTick = false;
    ActiveShakeInstance = nullptr;
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AWindManager::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    float WindInterval = FMath::RandRange(55.f, 65.f);

    GetWorldTimerManager().SetTimer(
        WindCycleTimerHandle,
        this,
        &AWindManager::OnWindWarning,
        WindInterval,
        false
    );

    GetWorldTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &AWindManager::LogCountdown,
        1.0f,
        true
    );
}

void AWindManager::OnWindWarning()
{
    if (!HasAuthority()) return;

    // 중앙 경고 UI 표시
    SetWindWarningForAllPlayers(true);

    // 카운트 시작
    StartWindWarningCountdown();

    OnWindWarningEvent();

    // 경고 3초 후 바람 발생
    GetWorldTimerManager().SetTimer(
        WindBlowTimerHandle,
        this,
        &AWindManager::OnWindBlow,
        3.0f,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("[WindManager] Warning! Wind in 3s"));
}

void AWindManager::OnWindBlow()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(WindWarningCountdownTimerHandle);

    // 바람 시작하면 경고 UI 숨김
    SetWindWarningForAllPlayers(false);

    if (!WindCameraShakeClass) return;

    MulticastStartShake();

    // 서버는 바람 종료 타이밍 관리
    GetWorldTimerManager().SetTimer(
        ShakeStopTimerHandle,
        this,
        &AWindManager::StopWindShake,
        WindShakeDuration,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("[WindManager] Wind Start - Intensity: %f"), WindIntensity);
}

void AWindManager::SetWindWarningForAllPlayers(bool bVisible)
{
    TArray<AActor*> PlayerActors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AVRPlayerCharacter::StaticClass(),
        PlayerActors
    );

    for (AActor* Actor : PlayerActors)
    {
        AVRPlayerCharacter* VRChar = Cast<AVRPlayerCharacter>(Actor);
        if (!VRChar) continue;

        VRChar->ClientSetWindWarningVisible(bVisible);
    }
}

void AWindManager::MulticastStartShake_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[WindManager] MulticastStartShake_Implementation called - Auth:%d"), (int)HasAuthority());

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Cyan,
            FString::Printf(
                TEXT("[Wind] Multicast received - PC:%s Class:%s Auth:%d"),
                PC ? TEXT("OK") : TEXT("NULL"),
                WindCameraShakeClass ? TEXT("OK") : TEXT("NULL"),
                (int)HasAuthority()
            )
        );
    }

    if (!WindCameraShakeClass) return;
    if (!PC || !PC->PlayerCameraManager) return;

    ActiveShakeInstance = PC->PlayerCameraManager->StartCameraShake(
        WindCameraShakeClass,
        WindIntensity
    );

    UE_LOG(LogTemp, Warning, TEXT("[WindManager] StartCameraShake called"));

    AVRPlayerCharacter* VRChar = Cast<AVRPlayerCharacter>(PC->GetPawn());
    if (VRChar)
    {
        VRChar->SetWindActive(true);
    }

    // 각 클라이언트에서 직접 WindShakeDuration 후 중단
    FTimerHandle LocalShakeStopHandle;
    TWeakObjectPtr<AWindManager> WeakThis(this);
    GetWorldTimerManager().SetTimer(
        LocalShakeStopHandle,
        [WeakThis]()
        {
            if (!WeakThis.IsValid()) return;
            AWindManager* Self = WeakThis.Get();

            APlayerController* LocalPC = UGameplayStatics::GetPlayerController(Self->GetWorld(), 0);
            if (LocalPC && LocalPC->PlayerCameraManager && Self->ActiveShakeInstance)
            {
                LocalPC->PlayerCameraManager->StopCameraShake(Self->ActiveShakeInstance, false);
                Self->ActiveShakeInstance = nullptr;
                AVRPlayerCharacter* StopChar = Cast<AVRPlayerCharacter>(LocalPC->GetPawn());
                if (StopChar)
                {
                    StopChar->SetWindActive(false);
                }
            }
        },
        WindShakeDuration,
        false
    );
}

void AWindManager::StopWindShake()
{
    if (!HasAuthority()) return;

    // 혹시 남아있을 수도 있으니 한 번 더 숨김
    SetWindWarningForAllPlayers(false);

    // 다음 바람 사이클
    GetWorldTimerManager().SetTimer(
        WindCycleTimerHandle,
        this,
        &AWindManager::OnWindWarning,
        110.0f,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("[WindManager] Wind Stop - Next warning in 3s"));
}

void AWindManager::MulticastStopShake_Implementation()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->PlayerCameraManager || !ActiveShakeInstance) return;

    PC->PlayerCameraManager->StopCameraShake(ActiveShakeInstance, false);
    ActiveShakeInstance = nullptr;
}

void AWindManager::LogCountdown()
{
    float WarnRemaining = GetWorldTimerManager().GetTimerRemaining(WindCycleTimerHandle);
    float BlowRemaining = GetWorldTimerManager().GetTimerRemaining(WindBlowTimerHandle);
    float StopRemaining = GetWorldTimerManager().GetTimerRemaining(ShakeStopTimerHandle);

    if (StopRemaining > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[WindManager] Wind stopping in: %.0fs"), StopRemaining);
    }
    else if (BlowRemaining > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[WindManager] Wind blowing in: %.0fs"), BlowRemaining);
    }
    else if (WarnRemaining > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[WindManager] Warning in: %.0fs"), WarnRemaining);
    }
}

void AWindManager::StartWindWarningCountdown()
{
    if (!HasAuthority()) return;

    CurrentWarningCountdown = 3;

    // 바로 3 표시
    SetWindWarningCountdownForAllPlayers(CurrentWarningCountdown);

    GetWorldTimerManager().SetTimer(
        WindWarningCountdownTimerHandle,
        this,
        &AWindManager::UpdateWindWarningCountdown,
        1.0f,
        true
    );
}

void AWindManager::UpdateWindWarningCountdown()
{
    if (!HasAuthority()) return;

    CurrentWarningCountdown--;

    if (CurrentWarningCountdown <= 0)
    {
        GetWorldTimerManager().ClearTimer(WindWarningCountdownTimerHandle);
        return;
    }

    SetWindWarningCountdownForAllPlayers(CurrentWarningCountdown);
}

void AWindManager::SetWindWarningCountdownForAllPlayers(int32 Count)
{
    TArray<AActor*> PlayerActors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AVRPlayerCharacter::StaticClass(),
        PlayerActors
    );

    for (AActor* Actor : PlayerActors)
    {
        AVRPlayerCharacter* VRChar = Cast<AVRPlayerCharacter>(Actor);
        if (!VRChar) continue;

        VRChar->ClientUpdateWindWarningCountdown(Count);
    }
}