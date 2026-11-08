// Fill out your copyright notice in the Description page of Project Settings.
// VR 인벤토리 컴포넌트 클래스
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRToolBase.h"	// [추가]
#include "VRPlayerState.h" // [추가]
#include "VR_WorkWatch.h"
#include "VRInventoryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VR_PROJECT_API UVRInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVRInventoryComponent();

	// 세탁기 블루프린트 클래스 칸
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Setup")
	TSubclassOf<class AVRToolBase> WasherClass;

	// 청소기 블루프린트 클래스 칸
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Setup")
	TSubclassOf<class AVRToolBase> CleanerClass;

	// 멀티플레이어 위한 복제화 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 복제화 변수
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLeftTool, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EToolType CurrentLeftTool;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentRightTool, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EToolType CurrentRightTool;

	UPROPERTY(Replicated)
	AVRToolBase* LeftToolActor;
	UPROPERTY(Replicated)
	AVRToolBase* RightToolActor;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Setup")
	TSubclassOf<AVR_WorkWatch> WatchClass;

	UFUNCTION()
	void OnRep_CurrentLeftTool();

	UFUNCTION()
	void OnRep_CurrentRightTool();

	// 도구 처리 알림 함수
	void HandleToolChangeNotification();

	// 외부(플레이어)에서 호출 시 인터페이스
	void SwitchToolInternal(EHandType Hand, bool bIsForward = true);
	void StartUseToolInternal(EHandType Hand);
	void StopUseToolInternal(EHandType Hand);
	void InteractInternal(EHandType Hand);

	class AVRTool_Washer* GetActiveWasher() const;

	// 도구 교체(스위칭) 할 때 물 양 수치 저장
	UPROPERTY()
	float StoredWasherWater;

	// 도구 잠금 해제 시 호출 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnlockTool(EToolType ToolToUnLock);
	void SaveToGameInstance();//추가
	void LoadFromGameInstance();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;//추가

	// 튜토리얼에서 도구 잠금 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Unlock")
	bool bIsWasherUnlocked = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Unlock")
	bool bIsCleanerUnlocked = false;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 블루프린트를 읽어들이고 앞에 붙여있는 도구 관리 함수
	void EquipTool(EToolType NewToolType, EHandType Hand);


};