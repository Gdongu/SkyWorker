// Fill out your copyright notice in the Description page of Project Settings.


#include "VRSafetyBase.h"

// Sets default values
AVRSafetyBase::AVRSafetyBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	SafetyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafetyMesh"));
	SafetyMesh->SetupAttachment(RootScene);
	SafetyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);	// 라인트레이스 검사에 걸리게
	SafetyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);	// 플레이어와 겹쳐지면 이동 움직임에 버그 발생(이상한 방향으로 움직임)
	SafetyMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

// Called when the game starts or when spawned
void AVRSafetyBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AVRSafetyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

