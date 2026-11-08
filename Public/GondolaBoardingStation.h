#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GondolaBoardingStation.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class AVRPlayerCharacter;

UCLASS()
class VR_PROJECT_API AGondolaBoardingStation : public AActor
{
	GENERATED_BODY()

public:
	AGondolaBoardingStation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어가 버튼을 눌렀을 때 호출
	UFUNCTION(BlueprintCallable, Category = "Gondola")
	void TryStartWorkLevel(AVRPlayerCharacter* InteractingPlayer);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> RootScene;

	// 플레이어 2명이 들어와야 하는 곤돌라 탑승 구역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> BoardingBox;

	// 손으로 누를 버튼 메시
	// 왼쪽 버튼 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> ButtonMesh_Left;

	// 오른쪽 버튼 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> ButtonMesh_Right;

	// 0/2, 1/2, 2/2 표시용 월드 UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> CountWidgetComponent;

	// 0/2, 1/2, 2/2 표시용 월드 UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> CountMiniRightWidgetComponent;
	// 0/2, 1/2, 2/2 표시용 월드 UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> CountMiniLeftWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gondola")
	int32 RequiredPlayerCount = 2;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerCount, BlueprintReadOnly, Category = "Gondola")
	int32 CurrentPlayerCount = 0;

	// 예: /Game/Levels/L_Work
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gondola")
	FString WorkLevelPath = TEXT("/Game/Levels/L_Work");

	// listen 서버 유지용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gondola")
	bool bAppendListenOption = true;

	UFUNCTION()
	void OnRep_CurrentPlayerCount();

	UFUNCTION()
	void OnBoardingBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnBoardingBoxEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void OnGondolaBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// [추가] 종소리 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	class USoundBase* SuccessBellSound;

	// [추가] 서버가 모든 클라이언트에게 사운드를 틀라고 지시하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySuccessBell();

	// [추가] 딜레이 후 실제로 맵을 이동시킬 함수와 타이머
	void ExecuteServerTravel();

	FTimerHandle TravelTimerHandle;

private:
	UPROPERTY()
	TSet<TObjectPtr<AVRPlayerCharacter>> BoardedPlayers;

	void RefreshBoardedPlayers();
	void UpdateCountWidget();


	//[추가] 중복 이동 방지 플래그
	bool bMovingToWork = false;
};