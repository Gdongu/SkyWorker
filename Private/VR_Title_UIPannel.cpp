// Fill out your copyright notice in the Description page of Project Settings.


#include "VR_Title_UIPannel.h"
#include "Components/WidgetComponent.h"
#include "VR_Title_LobbyWidget.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AVR_Title_UIPannel::AVR_Title_UIPannel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	VR_Origin = CreateDefaultSubobject<USceneComponent>(TEXT("VR_Origin"));
	RootComponent = VR_Origin;

	VR_Title_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("VR_TitleWidget"));
	VR_Title_Widget->SetupAttachment(VR_Origin);


	VR_Title_Widget->SetWidgetSpace(EWidgetSpace::World);
	VR_Title_Widget->SetDrawSize(FVector2D(1920.f, 1080.f));
	VR_Title_Widget->SetPivot(FVector2D(0.5f, 0.5f));
	VR_Title_Widget->SetTwoSided(true);

	VR_Title_Widget->SetBlendMode(EWidgetBlendMode::Transparent);
	VR_Title_Widget->SetBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
	VR_Title_Widget->SetWindowFocusable(false);

	//카메라 앞 거리 조절 (x : 카메라 앞)
	VR_Title_Widget->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	//크기 조절
	VR_Title_Widget->SetRelativeScale3D(FVector(0.08f));

}

// Called when the game starts or when spawned
void AVR_Title_UIPannel::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyWidgetClass && VR_Title_Widget) {
		VR_Title_Widget->SetWidgetClass(LobbyWidgetClass);
		VR_Title_Widget->InitWidget();
	}
}

