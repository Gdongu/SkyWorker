// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_Dirt.generated.h"

UCLASS()
class VR_PROJECT_API AVR_Dirt : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVR_Dirt();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 워셔에 닿았을 때 호출 할 체력 감소 함수
	UFUNCTION(BlueprintCallable, Category = "Dirt")
	void TakeCleaningDamage(float DamageAmount);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootScene;

	// 라인트레이스 or 오버랩 감지 투명 충돌체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBoxComponent;

	// 오염물 텍스처를 입힐 매쉬(Plane) 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* DirtMeshComponent;

	// 최대 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dirt|Status")
	float MaxHP = 100.f;

	// 현재 체력 (동기화)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly, Category = "Dirt|Status")
	float CurrentHP = 0.f;

	UFUNCTION()
	void OnRep_CurrentHP();

	// 투명도 조절위한 다이내믹 머티리얼 포인터
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMeshMaterial;

	UPROPERTY()
	bool bSpawnedDirtyWater = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Dirt")
	float GetCleaningProgress() const; // 청소 진행도 반환 (0.0 ~ 1.0)

	UFUNCTION(BlueprintPure, Category = "Dirt")
	bool IsCleaned() const; // 청소 완료 여부 반환

	void UpdateDirtVisuality();

	UFUNCTION(BlueprintPure, Category = "Dirt")
	bool HasSpawnedDirtyWater() const { return bSpawnedDirtyWater; }

	void MarkDirtyWaterSpawned() { bSpawnedDirtyWater = true; }
};
