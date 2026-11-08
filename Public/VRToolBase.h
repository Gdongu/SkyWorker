// Fill out your copyright notice in the Description page of Project Settings.
// 액터 상속받은 클래스
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRToolBase.generated.h"

UENUM(BlueprintType)
enum class EHandType : uint8
{
	Left	UMETA(DisplayName = "Left Hand"),
	Right	UMETA(DisplayName = "Right Hand")
};

UENUM(BlueprintType)
enum class EToolType : uint8
{
	None	UMETA(DisplayName = "Bare Hands"),
	Washer	UMETA(DisplayName = "Washer"),
	Cleaner	UMETA(DisplayName = "Cleaner")
};

UCLASS()
class VR_PROJECT_API AVRToolBase : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	AVRToolBase();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Conponents")
	class UStaticMeshComponent* ToolMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
	EToolType ToolType;

	virtual void StartUseTool();

	virtual void StopUseTool();
};
