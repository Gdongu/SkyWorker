// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SkyGameState.generated.h"

UCLASS()
class VR_PROJECT_API ASkyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	// 서버에서 경과 시간을 저장하는 변수 (서버→클라이언트 복제)
	UPROPERTY(ReplicatedUsing = OnRep_ElapsedSeconds, BlueprintReadOnly, Category = "Time")
	int32 ElapsedSeconds = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Progress")
	float SprayProgress = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Progress")
	float WipeProgress = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Progress")
	bool bWorkComplete = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gondola")
	int32 GondolaFloor = 0;

	// 경과 시간 변경 시 클라이언트에서 자동 호출
	UFUNCTION()
	void OnRep_ElapsedSeconds();

	// MM:SS 형식 문자열로 변환 (위젯/UI에서 바로 사용 가능)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Time")
	FText GetFormattedTime() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
