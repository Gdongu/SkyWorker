// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_Title_UIPannel.generated.h"

class UWidgetComponent;
class UVR_Title_LobbyWidget;
class UStaticMeshComponent;

UCLASS()
class VR_PROJECT_API AVR_Title_UIPannel : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVR_Title_UIPannel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	USceneComponent* VR_Origin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* VR_Title_Widget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVR_Title_LobbyWidget> LobbyWidgetClass;
};

//이 코드를 기반으로 BP 생성, VR_Title_Widget을 연결 (Rotation은 0, 0, -180)
//패널은 현재 Title맵에서 외부와 내부로 이어지는 길목에 생성함 (Scale을 작은 방이 안보이도록 함)
//추가로 Title맵에 있는 쓸모없는 Object들 전부 제거
//빛의 세기를 줄이고 약간 남색 빛이 돌게 하여 위젯의 배경과 비슷하도록 설정
//맵 천장 쪽에 큰 Plane을 하나 두고 하늘 Texture로 만든 Material을 적용시켜 하늘이 있는거 처럼 보이게 함 (이 작업은 필요하면 내가 직접 할거임)
