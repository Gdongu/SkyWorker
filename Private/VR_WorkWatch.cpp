#include "VR_WorkWatch.h"
#include "VRPlayerCharacter.h"
#include "VR_WorkWatchWidget.h"
#include "SkyGameState.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "MotionControllerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AVR_WorkWatch::AVR_WorkWatch()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	WatchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WatchMesh"));
	WatchMesh->SetupAttachment(RootScene);
	WatchMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WatchMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WatchMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WatchMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootScene);
	InteractionSphere->SetSphereRadius(35.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	WatchWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WatchWidgetComponent"));
	WatchWidgetComponent->SetupAttachment(WatchMesh);

	WatchWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
	WatchWidgetComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	WatchWidgetComponent->SetRelativeScale3D(FVector(0.03f, 0.03f, 0.03f));

	WatchWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WatchWidgetComponent->SetDrawSize(FVector2D(300.f, 300.f));
	WatchWidgetComponent->SetTwoSided(true);
}

void AVR_WorkWatch::BeginPlay()
{
	Super::BeginPlay();

	UpdateWatchWidgetFromGameMode();
}

void AVR_WorkWatch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 장착된 상태일 때만 계속 갱신.
	// 나중에 SkyGameModeBase에 데이터 넣으면 여기서 가져오면 됨.
	if (bEquipped)
	{
		UpdateWatchWidgetFromGameMode();
	}
}

void AVR_WorkWatch::EquipToPlayer(AVRPlayerCharacter* Player)
{
	if (!HasAuthority()) return;
	if (!Player) return;
	if (bEquipped) return;
	if (!Player->LeftHandController) return;

	EquippedPlayer = Player;
	bEquipped = true;

	AttachToComponent(
		Player->LeftHandController,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	SetActorRelativeLocation(EquippedRelativeLocation);
	SetActorRelativeRotation(EquippedRelativeRotation);
	SetActorRelativeScale3D(EquippedRelativeScale);

	ApplyEquippedVisual();
}

void AVR_WorkWatch::ApplyEquippedVisual()
{
	if (WatchMesh)
	{
		WatchMesh->SetSimulatePhysics(false);
		WatchMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		WatchMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		WatchMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (WatchWidgetComponent)
	{
		WatchWidgetComponent->SetVisibility(true);
	}
}

void AVR_WorkWatch::OnRep_Equipped()
{
	if (bEquipped)
	{
		ApplyEquippedVisual();
	}
}

void AVR_WorkWatch::UpdateWatchWidgetFromGameMode()
{
	if (!WatchWidgetComponent) return;

	UVR_WorkWatchWidget* WatchWidget =
		Cast<UVR_WorkWatchWidget>(WatchWidgetComponent->GetUserWidgetObject());

	if (!WatchWidget) return;

	ASkyGameState* GS = GetWorld()->GetGameState<ASkyGameState>();
	float SprayPart = GS ? GS->SprayProgress * 0.5f : 0.f;
	float CleanPart = GS ? GS->WipeProgress * 0.5f : 0.f;
	float TotalProgress = SprayPart + CleanPart;

	const FString TempZone = TEXT("A5");
	const float SyncedElapsedTime = GS ? static_cast<float>(GS->ElapsedSeconds) : 0.f;

	WatchWidget->UpdateWorkInfo(TotalProgress, TempZone, SyncedElapsedTime);
}

void AVR_WorkWatch::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVR_WorkWatch, bEquipped);
	DOREPLIFETIME(AVR_WorkWatch, EquippedPlayer);
}