// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRToolBase.h"	// [추가]
#include "InputActionValue.h"
#include "VRPlayerCharacter.generated.h"

class AGondola;
class USphereComponent;
class UWidgetInteractionComponent; // 튜토리얼 WBP 버튼 누르기 위해 추가
class UUserWidget;
class UStaticMeshComponent;
class UWidgetComponent;
class UVRPlayer_WaterWidget;

// 손 위치/회전 데이터
USTRUCT()
struct FVRHandData {
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize10 HandLocation;

	UPROPERTY()
	FRotator HandRotation;
};

UCLASS()
class VR_PROJECT_API AVRPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRPlayerCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 기본 VR 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UCameraComponent* VRCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* LeftHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* RightHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* RightHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	USphereComponent* LeftHandSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	USphereComponent* RightHandSphere;

	// 커스텀 컴포넌트 래퍼클래스(Safety, Inventory)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	class UVRInventoryComponent* InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	class UVRSafetyComponent* SafetyComponent;

	// 서버에서 물을 채워줄  함수
	UFUNCTION(Server, Reliable)
	void ServerRefillWasher(class AVRTool_Washer* Washer, float Amount);

	FTimerHandle BucketCheckTimerHandle;
	void CheckBucketWithGaze();

	// 인풋액션
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_SwitchLeftTool;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_SwitchRightTool;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_SwitchLeftToolReverse;	// 왼손 도구 역순 교체

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_SwitchRightToolReverse;	// 오른손 도구 역순 교체

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_UseLeftTool;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_UseRightTool;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_InteractLeft;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_InteractRight;

	// 테스트용 인풋액션
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Turn;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_LookUp;

	// PC 테스트용 곤돌라 키
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_TestGondolaUp;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_TestGondolaDown;

	// 테스트용 인풋액션
	void Turn(const FInputActionValue& Value);
	void LookUp(const FInputActionValue& Value);

	// 외부 입력 함수 인터페이스
	// 이동
	void Move(const FInputActionValue& Value);

	// 도구 교체
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwitchTool(EHandType Hand);

	// 도구 역순 변경
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwitchToolReverse(EHandType Hand);

	// 도구 사용 시작
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void StartUseTool(EHandType Hand);

	// 도구 사용 종료
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void StopUseTool(EHandType Hand);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(EHandType Hand);

	UFUNCTION(Client, Reliable)
	void ClientHideInteractedItem(AActor* ItemToHide);

	UPROPERTY(EditDefaultsOnly,Category="Sound")
	class USoundBase* EquipSound;

	// 인벤토리 컴포넌트에서 호출(체력바 등 기능 개선)
	void StartGazeTimer();
	void StopGazeTimer();
	void PressGondolaButton(AGondola* Gondola, int32 Dir);

	// PC 테스트용 곤돌라 함수
	void TestGondolaUp();
	void TestGondolaDown();

protected:
	// 서버 RPC 함수 선언
	UFUNCTION(Server, Reliable)
	void ServerSwitchTool(EHandType Hand);

	UFUNCTION(Server, Reliable)
	void ServerSwitchToolReverse(EHandType Hand);

	UFUNCTION(Server, Reliable)
	void ServerStartUseTool(EHandType Hand);

	UFUNCTION(Server, Reliable)
	void ServerStopUseTool(EHandType Hand);

	UFUNCTION(Server, Reliable)
	void ServerInteract(EHandType Hand);

	UFUNCTION(Server, Reliable)
	void ServerPressGondolaButton(AGondola* Gondola, int32 Dir);

	UFUNCTION(Server, Reliable)
	void ServerTestGondolaUp();

	UFUNCTION(Server, Reliable)
	void ServerTestGondolaDown();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void SetWindActive(bool Active);

protected:
	UFUNCTION(Server, Reliable)
	void ServerWage();

	void CheckMovementDuringWind();

	bool bWindActive = false;
	bool bDeductedThisWind = false;
	FTimerHandle WindCheckTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Widget")
	UWidgetInteractionComponent* LeftWidgetInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Widget")
	UWidgetInteractionComponent* RightWidgetInteraction;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Watch")
	UStaticMeshComponent* WatchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Watch")
	UWidgetComponent* WatchWidget;

	UFUNCTION(BlueprintCallable, Category = "Watch")
	void SetWatchVisible(bool bVisible);

	// 위젯 상호작용 함수
	void PressWidgetButton(EHandType Hand);
	void ReleaseWidgetButton(EHandType Hand);

	// 바람 경고 UI 표시 함수
	UFUNCTION(Client, Reliable)
	void ClientSetWindWarningVisible(bool bVisible);

	// 바람 경고 카운트다운 업데이트 함수
	UFUNCTION(Client, Reliable)
	void ClientUpdateWindWarningCountdown(int32 Count);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Wind")
	TSubclassOf<UUserWidget> WindWarningWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> WindWarningWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Wind")
	UWidgetComponent* WindWarningWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|WorkComplete")
	TSubclassOf<UUserWidget> WorkCompleteWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|WorkComplete")
	UWidgetComponent* WorkCompleteWidgetComponent;

	UFUNCTION(Client, Reliable)
	void ClientShowWorkCompleteWidget();

	void UpdateWatchWidget();
	FTimerHandle WatchUpdateTimerHandle;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> WaterHitWidgetComponent;

	UPROPERTY()
	TObjectPtr<UVRPlayer_WaterWidget> ActiveWaterHitWidget;

	UFUNCTION(Client, Reliable)
	void Client_StartWaterHitImage();

	FTimerHandle WaterHitEffectTimerHandle;

	void UpdateWaterHitEffectByTimer();

	// 손 컨트롤러 위치 동기화
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Unreliable)
	void ServerSyncHandTransforms(FTransform LeftTransform, FTransform RightTransform);

	UPROPERTY(Replicated)
	FTransform ReplicatedLeftHandTransform;

	UPROPERTY(Replicated)
	FTransform ReplicatedRightHandTransform;

	// 손 위치 Replication 시키기
public:
	//서버 Replication용 손 데이터
	UPROPERTY(ReplicatedUsing = OnRep_VRHands)
	FVRHandData ReplicatedLeftHand;

	UPROPERTY(ReplicatedUsing = OnRep_VRHands)
	FVRHandData ReplicatedRightHand;

	UFUNCTION(Server, Unreliable)
	void ServerUpdateVRHandData(const FVRHandData& LeftHandData, const FVRHandData& RightHandData);

	UFUNCTION()
	void OnRep_VRHands();

	FVRHandData MakeHandReplicatedData(UMotionControllerComponent* HandComp);
	void ApplyHandReplicatedData(UMotionControllerComponent* HandComp, const FVRHandData& Data);

	float HandReplicationSendTimer = 0.f;

	//게임 종료 후 타이틀 화면으로 복귀 관련
	UFUNCTION(BlueprintCallable)
	void RequestReturnAllPlayersToTitle();

	UFUNCTION(Server, Reliable)
	void ServerRequestReturnAllPlayersToTitle();

	UFUNCTION(Client, Reliable)
	void ClientReturnToTitle();

	void LocalReturnToTitle();
};
