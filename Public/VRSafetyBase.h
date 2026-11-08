// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRSafetyBase.generated.h"

UCLASS()
class VR_PROJECT_API AVRSafetyBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRSafetyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Conponents")
	class UStaticMeshComponent* SafetyMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
