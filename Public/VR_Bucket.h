// Fill out your copyright notice in the Description page of Project Settings.
// 액터 상속받은 클래스
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_Bucket.generated.h"

class UWidgetComponent;
class UUserWidget;

UCLASS()
class VR_PROJECT_API AVR_Bucket : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVR_Bucket();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class UStaticMeshComponent* BucketMesh;

	// 물 채우기 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UWidgetComponent* RefillWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Refill")
	TSubclassOf<UUserWidget> RefillWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Bucket|Refill")
	void ShowRefillWidget(float CurrentWater, float MaxWater);

	UFUNCTION(BlueprintCallable, Category = "Bucket|Refill")
	void HideRefillWidget();

	FTimerHandle AttachTimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle HideRefillWidgetTimerHandle;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
