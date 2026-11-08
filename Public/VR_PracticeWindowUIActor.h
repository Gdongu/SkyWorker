#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_PracticeWindowUIActor.generated.h"

class UBoxComponent;
class UWidgetComponent;
class AVR_Dirt;
class AVR_WaterResidue;

UCLASS()
class VR_PROJECT_API AVR_PracticeWindowUIActor : public AActor
{
	GENERATED_BODY()

public:
	AVR_PracticeWindowUIActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> RootScene;

	// 이 박스 안에 있는 Dirt / DirtyWater만 연습 UI 계산에 포함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> WindowAreaBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> PracticeWidgetComponent;

	// BeginPlay 때 창문에 붙어있는 먼지들을 저장
	UPROPERTY()
	TArray<TObjectPtr<AVR_Dirt>> TargetDirtActors;

	// 워셔로 생성되는 구정물들을 실시간으로 저장
	UPROPERTY()
	TArray<TObjectPtr<AVR_WaterResidue>> TrackedDirtyWaters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Practice UI")
	float UpdateInterval = 0.05f;

	FTimerHandle UpdateTimerHandle;

	void UpdateTrackedDirtActors();
	void UpdateTrackedDirtyWaters();
	void UpdatePracticeUI();

	float CalculateSprayProgress() const;
	float CalculateWipeProgress() const;

	// 연습 완료 후 5초 뒤 초기화
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Practice UI|Reset")
	float ResetDelay = 5.f;

	// 처음 배치된 Dirt 정보를 저장해서 리셋 때 다시 스폰
	UPROPERTY()
	TArray<TSubclassOf<AVR_Dirt>> InitialDirtClasses;

	UPROPERTY()
	TArray<FTransform> InitialDirtTransforms;

	FTimerHandle ResetPracticeTimerHandle;

	bool bResetScheduled = false;
	bool bInitialDirtSaved = false;

	void SaveInitialDirtSpawnData();
	void ScheduleResetIfCompleted(float SprayProgress, float WipeProgress);
	void ResetPracticeWindow();
};