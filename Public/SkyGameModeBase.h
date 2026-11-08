// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SkyGameModeBase.generated.h"

class AWindManager;

UCLASS()
class VR_PROJECT_API ASkyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASkyGameModeBase();

	int32 WipeWaterCount = 0;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Managers")
	TSubclassOf<AWindManager> WindManagerClass;

private:
	FTimerHandle ElapsedTimerHandle;

	// 1초마다 호출되어 경과 시간 증가
	void IncrementElapsedTime();

	TArray<AActor*> CachedDirtActors;
	int32 TotalDirtCount = 0;


	//[추가] 타이틀 화면 복귀
public:
	UPROPERTY()
	bool bReturningToTitle = false;

	void ReturnAllPlayersToTitle();

	void ReturnHostToTitle();
};