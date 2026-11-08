// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindManager.generated.h"

class UCameraShakeBase;

UCLASS()
class VR_PROJECT_API AWindManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWindManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnWindWarning();
	void OnWindBlow();
	void StopWindShake();
	void LogCountdown();

	// 플레이어들에게 경고 UI 표시/숨김
	void SetWindWarningForAllPlayers(bool bVisible);

	//블루프린트에서 경고 연출 구현 (사운드 ui)
	UFUNCTION(BlueprintImplementableEvent, Category = "Wind")
	void OnWindWarningEvent();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartShake();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopShake();

	UPROPERTY(EditAnywhere, Category = "Wind Settings")
	TSubclassOf<UCameraShakeBase> WindCameraShakeClass;
	UPROPERTY(EditAnywhere, Category = "Wind Settings")
	float WindIntensity = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Wind Settings")
	float WindShakeDuration = 5.0f;

	void StartWindWarningCountdown();
	void UpdateWindWarningCountdown();
	void SetWindWarningCountdownForAllPlayers(int32 Count);

private:
	FTimerHandle WindCycleTimerHandle;
	FTimerHandle WindBlowTimerHandle;
	FTimerHandle ShakeStopTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle WindWarningCountdownTimerHandle;

	UPROPERTY()
	UCameraShakeBase* ActiveShakeInstance;

	int32 CurrentWarningCountdown = 3;
};
