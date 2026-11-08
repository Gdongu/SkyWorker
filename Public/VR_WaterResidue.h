// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_WaterResidue.generated.h"

UCLASS()
class VR_PROJECT_API AVR_WaterResidue : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVR_WaterResidue();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Water")
	void TakeWipingDamage(float DamageAmount);

	// 물기를 처음 생성할 때 상태를 결정해 줄 함수
	void SetupWater(bool bInIsDirty);

	// 물기가 구정물인지 여부(클리너로 구정물만 지울거임)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Water")
	bool bIsDirty;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WaterMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water|Status")
	float MaxHP;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly, Category = "Water|Status")
	float CurrentHP;

	UFUNCTION()
	void OnRep_CurrentHP();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Water|Status")
	float Lifetime = 3.f;

	UPROPERTY(ReplicatedUsing = OnRep_NowLifetime, BlueprintReadOnly, Category = "Water|Status")
	float NowLifeTime = 0.f;

	UFUNCTION()
	void OnRep_NowLifetime();

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicWaterMaterial;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FTimerHandle LifeTimeChanger;

	UFUNCTION(BlueprintPure, Category = "Water")
	float GetWipingProgress() const; // 물기 제거 진행도 반환 (0.0 ~ 1.0)

	UFUNCTION(BlueprintPure, Category = "Water")
	bool IsWiped() const; // 물기 제거 완료 여부 반환

	void LifeTimeChange();

	void UpdateDirtVisuality();

private:
	bool bWipeCounted = false;
};
