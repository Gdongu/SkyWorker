// Fill out your copyright notice in the Description page of Project Settings.
// 액터컴포넌트 상속받은 클래스
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRToolBase.h"
#include "VRSafetyComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VR_PROJECT_API UVRSafetyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVRSafetyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 안전 장비 동기화 변수
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Safety")
	bool bHasHelmet;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Safety")
	bool bHasHarness;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Safety")
	bool bHasHook;

	UPROPERTY(ReplicatedUsing = OnRep_HasWatch, BlueprintReadWrite, Category = "Safety")
	bool bHasWatch;

	UFUNCTION()
	void OnRep_HasWatch();

	// 외부에서 호출 할 상호작용 로직
	void InteractInternal(EHandType Hand);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
