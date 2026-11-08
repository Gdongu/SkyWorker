// Fill out your copyright notice in the Description page of Project Settings.
// VRToolBase 상속받은 클래스
#pragma once

#include "CoreMinimal.h"
#include "VRToolBase.h"
#include "VRTool_Cleaner.generated.h"

UCLASS()
class VR_PROJECT_API AVRTool_Cleaner : public AVRToolBase
{
	GENERATED_BODY()

public:
	AVRTool_Cleaner();

protected:
	virtual void BeginPlay() override;

	// 메쉬 상단에 장착하여 창문의 오염물과의 접촉을 감지할 박스 충돌체 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class UBoxComponent* CleanerBoxCollision;

	// 창문에 처음 닿는 순간 감지하는 함수
	UFUNCTION()
	void OnCleanerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 창문에서 클리너가 떨어지는 순간을 감지하는 함수
	UFUNCTION()
	void OnCleanerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 닿아있는 동안 0.1초 마다 지속적으로 호출 될 청소 연산 함수
	void ProcessCleaning();

	// 타이머핸들
	FTimerHandle CleaningTimerHandle;

	// 현재 닿아있는 창문액터 기억할 포인터
	UPROPERTY()
	AActor* CurrentTargetWindow;

	// 0.1초 전의 클리너 위치 기억할 변수
	UPROPERTY()
	FVector LastCleanerLocation;

	// 서버에서 청소 진행도 올려줄 RPC 함수
	UFUNCTION(Server, Reliable)
	void ServerProgressCleaning(AActor* TargetWindow, float Amount);

	// 사운드 관련 선언
	// 동기화
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 멀티플레이 동기화용 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsWiping, BlueprintReadOnly, Category = "CleanerState")
	bool bIsWiping;
	// 내가 닦고있는지 체크하는 로컬 변수
	bool bLocalIsWiping;

	UFUNCTION()
	void OnRep_IsWiping();
	// 블루프린트에서 작성할 이벤트 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "CleanerEffect")
	void K2_OnWipingChanged(bool bNewIsWiping);
	// 클라이언트에서 서버로 전달 할 함수
	UFUNCTION(Server, Reliable)
	void ServerSetWiping(bool bWiping);



};
