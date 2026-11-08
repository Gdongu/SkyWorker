#include "GondolaBoardingStation.h"
#include "VRPlayerCharacter.h"
#include "VR_GondolaCountWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

//추가
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGondolaBoardingStation::AGondolaBoardingStation()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	BoardingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingBox"));
	BoardingBox->SetupAttachment(RootScene);
	BoardingBox->SetBoxExtent(FVector(150.f, 150.f, 120.f));
	BoardingBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoardingBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardingBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoardingBox->SetGenerateOverlapEvents(true);

	// 왼쪽 버튼
	ButtonMesh_Left = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh_Left"));
	ButtonMesh_Left->SetupAttachment(RootScene);
	ButtonMesh_Left->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ButtonMesh_Left->SetCollisionResponseToAllChannels(ECR_Ignore);
	ButtonMesh_Left->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ButtonMesh_Left->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ButtonMesh_Left->SetRelativeLocation(FVector(0.f, -30.f, 100.f));	

	// 오른쪽 버튼
	ButtonMesh_Right = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh_Right"));
	ButtonMesh_Right->SetupAttachment(RootScene);
	ButtonMesh_Right->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ButtonMesh_Right->SetCollisionResponseToAllChannels(ECR_Ignore);
	ButtonMesh_Right->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ButtonMesh_Left->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ButtonMesh_Right->SetRelativeLocation(FVector(0.f, 30.f, 100.f));

	CountWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CountWidgetComponent"));
	CountWidgetComponent->SetupAttachment(RootScene);
	CountWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	CountWidgetComponent->SetDrawSize(FVector2D(300.f, 100.f));
	CountWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
	CountWidgetComponent->SetTwoSided(true);

	CountMiniRightWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CountRightWidgetComponent"));
	CountMiniRightWidgetComponent->SetupAttachment(RootScene);
	CountMiniRightWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	CountMiniRightWidgetComponent->SetDrawSize(FVector2D(300.f, 100.f));
	CountMiniRightWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
	CountMiniRightWidgetComponent->SetTwoSided(true);

	CountMiniLeftWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CountLeftWidgetComponent"));
	CountMiniLeftWidgetComponent->SetupAttachment(RootScene);
	CountMiniLeftWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	CountMiniLeftWidgetComponent->SetDrawSize(FVector2D(300.f, 100.f));
	CountMiniLeftWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
	CountMiniLeftWidgetComponent->SetTwoSided(true);
}

void AGondolaBoardingStation::BeginPlay()
{
	Super::BeginPlay();

	if (BoardingBox)
	{
		BoardingBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ThisClass::OnBoardingBoxBeginOverlap
		);

		BoardingBox->OnComponentEndOverlap.AddDynamic(
			this,
			&ThisClass::OnBoardingBoxEndOverlap
		);
	}

	if (ButtonMesh_Left) {
		ButtonMesh_Left->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnGondolaBeginOverlap);
	}
	if (ButtonMesh_Right) {
		ButtonMesh_Right->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnGondolaBeginOverlap);
	}

	//[추가]
	bMovingToWork = false;

	UpdateCountWidget();
}

void AGondolaBoardingStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGondolaBoardingStation, CurrentPlayerCount);
}

void AGondolaBoardingStation::OnBoardingBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority()) return;

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(OtherActor);
	if (!Player) return;

	BoardedPlayers.Add(Player);
	RefreshBoardedPlayers();
}

void AGondolaBoardingStation::OnBoardingBoxEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!HasAuthority()) return;

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(OtherActor);
	if (!Player) return;

	BoardedPlayers.Remove(Player);
	RefreshBoardedPlayers();
}

void AGondolaBoardingStation::OnGondolaBeginOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(OtherActor);
	if (!Player) return;

	TryStartWorkLevel(Player);
}

void AGondolaBoardingStation::RefreshBoardedPlayers()
{
	if (!HasAuthority()) return;

	for (auto It = BoardedPlayers.CreateIterator(); It; ++It)
	{
		if (!IsValid(*It))
		{
			It.RemoveCurrent();
		}
	}

	CurrentPlayerCount = BoardedPlayers.Num();

	// 서버 자기 화면도 바로 갱신
	UpdateCountWidget();
}

void AGondolaBoardingStation::OnRep_CurrentPlayerCount()
{
	UpdateCountWidget();
}

void AGondolaBoardingStation::UpdateCountWidget()
{
	if (!CountWidgetComponent || !CountMiniRightWidgetComponent || !CountMiniLeftWidgetComponent) return;

	UVR_GondolaCountWidget* CountWidget =
		Cast<UVR_GondolaCountWidget>(CountWidgetComponent->GetUserWidgetObject());
	UVR_GondolaCountWidget* CountLeftWidget =
		Cast<UVR_GondolaCountWidget>(CountMiniRightWidgetComponent->GetUserWidgetObject());
	UVR_GondolaCountWidget* CountRightWidget =
		Cast<UVR_GondolaCountWidget>(CountMiniLeftWidgetComponent->GetUserWidgetObject());

	if (!CountWidget || !CountLeftWidget || !CountRightWidget) return;

	CountWidget->SetCountText(CurrentPlayerCount, RequiredPlayerCount);
	CountLeftWidget->SetCountText(CurrentPlayerCount, RequiredPlayerCount);
	CountRightWidget->SetCountText(CurrentPlayerCount, RequiredPlayerCount);
}

void AGondolaBoardingStation::TryStartWorkLevel(AVRPlayerCharacter* InteractingPlayer)
{
	if (!HasAuthority()) return;
	if (!InteractingPlayer) return;
	//[추가]중복 이동 방지
	if (bMovingToWork) return;

	// 버튼 누른 플레이어도 실제로 탑승 구역 안에 있어야 함
	if (!BoardedPlayers.Contains(InteractingPlayer))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Red,
				TEXT("[Gondola] 버튼은 곤돌라 안에서만 누를 수 있음")
			);
		}
		return;
	}

	if (CurrentPlayerCount < RequiredPlayerCount)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Red,
				FString::Printf(TEXT("[Gondola] 인원 부족: %d/%d"), CurrentPlayerCount, RequiredPlayerCount)
			);
		}
		return;
	}

	if (WorkLevelPath.IsEmpty()) return;

	//[추가] 중복 이동 방지
	bMovingToWork = true;

	if (CountWidgetComponent && CountMiniRightWidgetComponent && CountMiniLeftWidgetComponent) {
		UVR_GondolaCountWidget* CountWidget =
			Cast<UVR_GondolaCountWidget>(CountWidgetComponent->GetUserWidgetObject());
		UVR_GondolaCountWidget* CountLeftWidget =
			Cast<UVR_GondolaCountWidget>(CountMiniRightWidgetComponent->GetUserWidgetObject());
		UVR_GondolaCountWidget* CountRightWidget =
			Cast<UVR_GondolaCountWidget>(CountMiniLeftWidgetComponent->GetUserWidgetObject());
		if (CountWidget && CountLeftWidget && CountRightWidget) {
			CountWidget->bIsLoading = true;
			CountLeftWidget->bIsLoading = true;
			CountRightWidget->bIsLoading = true;
			CountWidget->SetLoadingText();
			CountLeftWidget->SetLoadingText();
			CountRightWidget->SetLoadingText();
		}
	}

	/*FString TravelURL = WorkLevelPath;

	if (bAppendListenOption)
	{
		TravelURL += TEXT("?listen");
	}

	GetWorld()->ServerTravel(TravelURL);*/

	// 사운드 재생 후, 레벨이동 함수는 타이머를 걸고 사운드 재생 후 이동하도록 변경
	MulticastPlaySuccessBell();

	GetWorld()->GetTimerManager().SetTimer(TravelTimerHandle, this, &AGondolaBoardingStation::ExecuteServerTravel, 1.1f, false);	// 사운드재생시간 1초여서 1.1초 딜레이 적용
}


void AGondolaBoardingStation::MulticastPlaySuccessBell_Implementation()
{
	if (SuccessBellSound)
	{
		// 맵 어디에 있든 똑같이 들리도록 2D 사운드로 재생
		UGameplayStatics::PlaySound2D(GetWorld(), SuccessBellSound);
	}
}

void AGondolaBoardingStation::ExecuteServerTravel()
{
	if (!HasAuthority()) return;
	if (WorkLevelPath.IsEmpty()) return;

	FString TravelURL = WorkLevelPath;

	if (bAppendListenOption)
	{
		TravelURL += TEXT("?listen");
	}

	// 다음 맵으로 이동
	GetWorld()->ServerTravel(TravelURL);
}


