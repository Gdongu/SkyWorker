// Fill out your copyright notice in the Description page of Project Settings.
// 액터 상속받은 클래스

#include "VRToolBase.h"

// Sets default values
AVRToolBase::AVRToolBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	ToolMesh->SetupAttachment(RootScene);
	ToolMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);	// 라인트레이스 검사에 걸리게
	ToolMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);	// 플레이어와 겹쳐지면 이동 움직임에 버그 발생(이상한 방향으로 움직임)
	ToolMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	ToolType = EToolType::None;

}

void AVRToolBase::StartUseTool()
{
}

void AVRToolBase::StopUseTool()
{
}



