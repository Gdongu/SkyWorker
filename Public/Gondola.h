// Fill out your copyright notice in the Description page of Project Settings.

#pragma once  // 헤더 중복 포함 방지

#include "CoreMinimal.h"           // UE 기본 타입 (FVector, int32 등)
#include "GameFramework/Actor.h"   // AActor 상속
#include "Gondola.generated.h"     // UE 리플렉션 자동 생성 헤더 (UCLASS, UPROPERTY 등 사용 시 필수)

class UBoxComponent;       // 전방 선언: 포인터만 쓰므로 헤더 include 불필요
class AVRPlayerCharacter;  // 전방 선언: 탑승자 추적용

class UBoxComponent;
class UWidgetComponent;
class UUserWidget;
class AVRPlayerCharacter;

UCLASS()  // 이 클래스를 UE 리플렉션/에디터/블루프린트에 등록
class VR_PROJECT_API AGondola : public AActor  // VRPROJECTTEST_API: DLL 익스포트 매크로
{
	GENERATED_BODY()  // 리플렉션에 필요한 코드를 UE가 자동 삽입

public:
	AGondola();  // 생성자: 컴포넌트 생성 및 기본값 설정

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")  // 에디터 뷰포트에서 보임, BP 읽기전용
		UStaticMeshComponent* GondolaMesh;  // 곤돌라 본체 메시, RootComponent로 사용

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* UpButtonMesh;  // 위 버튼 시각적 메시, GondolaMesh에 부착

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DownButtonMesh;  // 아래 버튼 시각적 메시, GondolaMesh에 부착

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* UpOverlapBox;  // 위 버튼 감지 박스, VR 손이 닿으면 GoUp 발동

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DownOverlapBox;  // 아래 버튼 감지 박스, VR 손이 닿으면 GoDown 발동

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* UpButtonMesh2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DownButtonMesh2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* UpOverlapBox2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DownOverlapBox2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* WorkCompleteWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|WorkComplete")
	TSubclassOf<UUserWidget> WorkCompleteWidgetClass;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowWorkCompleteWidget(float Money, int32 ElapsedSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* FloorBox;  // 탑승자 감지용 Overlap 박스 (물리 블록 아님), 바닥 위에 위치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gondola")  // 에디터/BP에서 읽기쓰기 가능
		int32 NumFloors = 5;  // 최상층에서 내려갈 수 있는 총 층 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gondola")
	float FloorHeight = 250.f;  // 층간 거리 (cm), 300 = 3m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gondola")
	float MoveSpeed = 100.f;  // 이동 속도 (cm/s), FInterpConstantTo에서 사용

	void GoUp();    // 한 층 위로 이동 요청: CurrentFloor-- 후 TargetZ 갱신
	void GoDown();  // 한 층 아래로 이동 요청: CurrentFloor++ 후 TargetZ 갱신

protected:
	virtual void BeginPlay() override;            // 게임 시작 시: TopWorldZ 저장, 오버랩 이벤트 바인딩
	virtual void Tick(float DeltaTime) override;  // 매 프레임: 곤돌라 이동 처리 + 탑승자 위치 동기화

	UFUNCTION()  // 델리게이트(AddDynamic)에 바인딩하려면 UFUNCTION 필수
		void OnUpOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);  // 위 버튼 박스 오버랩 시작 콜백

	UFUNCTION()
	void OnDownOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);  // 아래 버튼 박스 오버랩 시작 콜백

	UFUNCTION()
	void OnUpOverlapBegin2(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDownOverlapBegin2(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);  // 캐릭터가 FloorBox에 진입 → Passengers 추가

	UFUNCTION()
	void OnFloorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);  // EndOverlap은 bFromSweep 파라미터 없음

	// [추가] 곤돌라 작동 경고음 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	class USoundBase* MoveWarningSound;

	// [추가] 모두에게 경고음을 재생하라는 방송 함수
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMoveWarningSound();

private:
	float TopWorldZ;          // BeginPlay에서 저장한 최상층 Z 좌표 (배치 위치 기준)
	float TargetZ;            // 현재 이동 목표 Z 좌표, GoUp/GoDown 호출 시 갱신
	int32 CurrentFloor = 0;   // 현재 층 번호 (0 = 최상층, NumFloors = 최하층)
	TArray<AVRPlayerCharacter*> Passengers;  // 현재 곤돌라에 탑승 중인 캐릭터 목록

	bool bIsMoving = false;

	void SetButtonCollisionEnabled(bool bEnabled);
	bool CanOperateButton() const;
	void StartGondolaMoveToFloor(int32 NewFloor);
};
