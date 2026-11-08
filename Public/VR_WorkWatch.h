#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_WorkWatch.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class USphereComponent;
class AVRPlayerCharacter;

UCLASS()
class VR_PROJECT_API AVR_WorkWatch : public AActor
{
	GENERATED_BODY()

public:
	AVR_WorkWatch();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Work Watch")
	void EquipToPlayer(AVRPlayerCharacter* Player);

	UFUNCTION(BlueprintPure, Category = "Work Watch")
	bool IsEquipped() const { return bEquipped; }

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> WatchMesh;

	// 시계 유리 부분에 붙일 WBP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> WatchWidgetComponent;

	// 월드에서 상호작용 판정용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> InteractionSphere;

	// 왼손 컨트롤러에 붙었을 때 상대 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work Watch|Attach")
	FVector EquippedRelativeLocation = FVector(0.f, -6.f, -2.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work Watch|Attach")
	FRotator EquippedRelativeRotation = FRotator(0.f, 90.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work Watch|Attach")
	FVector EquippedRelativeScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(ReplicatedUsing = OnRep_Equipped)
	bool bEquipped = false;

	UPROPERTY(Replicated)
	TObjectPtr<AVRPlayerCharacter> EquippedPlayer;

	UFUNCTION()
	void OnRep_Equipped();

	void ApplyEquippedVisual();
	void UpdateWatchWidgetFromGameMode();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AVRPlayerCharacter* GetEquippedPlayer() const { return EquippedPlayer; }
};