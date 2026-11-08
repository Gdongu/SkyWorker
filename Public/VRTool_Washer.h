// Fill out your copyright notice in the Description page of Project Settings.
// VRToolBase 상속받은 클래스
#pragma once

#include "CoreMinimal.h"
#include "VRToolBase.h"
#include "VRTool_Washer.generated.h"

UCLASS()
class VR_PROJECT_API AVRTool_Washer : public AVRToolBase
{
	GENERATED_BODY()

public:
	AVRTool_Washer();

	// 멀티플레이 동기화
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void StartUseTool() override;
	virtual void StopUseTool() override;

	// 물 보충 함수
	UFUNCTION(BlueprintCallable, Category = "Washer|Water")
	void RefillWater(float Amount);

	// 현재 수압 배율 반환
	UFUNCTION(BlueprintPure, Category = "Washer|Water")
	float GetWaterPressureMultiplier() const;

	// 남은 물의 양
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Washer|Water")
	float CurrentWater;

	// 최대 물 용량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Washer|Water")
	float MaxWater;

	// 물 보충 사운드 구현 위한 변수
	// 내가 충전 중인지 체크하는 로컬 변수 (네트워크 과부하 방지)
	bool bLocalIsRefilling;

	// 클라이언트 -> 서버로 상태 전달
	UFUNCTION(Server, Reliable)
	void ServerSetRefilling(bool bRefilling);

protected:

	// 초당 물 소모량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Washer|Water")
	float WaterConsumptionRate;

	// 현재 물 발사중인지 여부
	UPROPERTY(ReplicatedUsing = OnRep_IsSpraying, BlueprintReadOnly, Category = "Washer|State")
	bool bIsSpraying;

	// 블루프린트에서 "발사" 이펙트나 사운드를 제어하기 편하도록 이벤트 열어둠(머지충돌발지)
	UFUNCTION(BlueprintImplementableEvent, Category = "Washer|Effects")
	void K2_OnSprayingChanged(bool bNewIsSpraying);

	// 클라이언트들이 물 발사 이펙트.사운드 켜고 끄기 위한 OnRep함수
	UFUNCTION()
	void OnRep_IsSpraying();

	//----------------물 보충 사운드------------------------
	// 멀티플레이 동기화용 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsRefilling)
	bool bIsRefilling;

	UFUNCTION()
	void OnRep_IsRefilling();

	// 블루프린트로 토스할 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "WasherEffects")
	void K2_OnRefillingChanged(bool bNewIsRefilling);


	// 블루프린트에서 스폰 할 물기 액터 지정할 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Washer|Setup")
	TSubclassOf<class AVR_WaterResidue> WaterResidueClass;

	// 마지막 물기를 생성한 위치 기억 -> 겹침 스폰 방지용
	UPROPERTY()
	FVector LastWaterSpawnLocation;

	// 구정물 마지막 위치 기억
	UPROPERTY()
	FVector LastDirtyWaterSpawnLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Washer|Setup")
	float WaterSpawnDistanceThreshold = 40.f;

private:
	// 타이머가 돌면서 주기적으로 호출할 물 소모 함수
	void ConsumeWater();

	// 타이머 핸들
	FTimerHandle WaterTimerHandle;

	// 타이머 주기
	const float WaterTimerInterval = 0.05f;

};
