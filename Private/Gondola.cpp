#include "Gondola.h"
#include "Components/StaticMeshComponent.h"  // UStaticMeshComponent 사용
#include "Components/BoxComponent.h"          // UBoxComponent 사용
#include "Components/SphereComponent.h"       // 버튼 오버랩 필터링: USphereComponent 캐스트용
#include "VRPlayerCharacter.h"         // AVRPlayerCharacter 캐스트, PressGondolaButton 호출용

#include "Components/WidgetComponent.h"
#include "VR_WorkCompleteWidget.h"
#include "SkyGameState.h"
#include "VRPlayerState.h"
#include "Kismet/GameplayStatics.h"

AGondola::AGondola()
{
	PrimaryActorTick.bCanEverTick = true;  // Tick 활성화 (매 프레임 이동 처리에 필요)
	bReplicates = true;                     // 이 액터를 서버에서 클라이언트로 복제
	SetReplicateMovement(true);             // 위치/회전 변경도 자동 복제 (클라이언트에서 이동 반영)

	GondolaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GondolaMesh"));  // 본체 메시 생성
	GondolaMesh->SetMobility(EComponentMobility::Movable);  // Tick에서 위치 변경 시 Movable이어야 경고 없음
	RootComponent = GondolaMesh;  // 루트로 설정: 모든 자식 컴포넌트의 기준점

	UpButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpButtonMesh"));  // 위 버튼 메시 생성
	UpButtonMesh->SetupAttachment(GondolaMesh);  // GondolaMesh에 부착: 곤돌라 이동 시 함께 이동

	DownButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DownButtonMesh"));  // 아래 버튼 메시 생성
	DownButtonMesh->SetupAttachment(GondolaMesh);  // GondolaMesh에 부착

	UpOverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("UpOverlapBox"));  // 위 버튼 감지 박스 생성
	UpOverlapBox->SetupAttachment(UpButtonMesh);           // 버튼 메시에 부착
	UpOverlapBox->SetBoxExtent(FVector(8.f, 8.f, 8.f));   // 반경 8cm (총 16x16x16cm), 버튼 크기에 맞게

	DownOverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DownOverlapBox"));  // 아래 버튼 감지 박스 생성
	DownOverlapBox->SetupAttachment(DownButtonMesh);
	DownOverlapBox->SetBoxExtent(FVector(8.f, 8.f, 8.f));

	UpButtonMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpButtonMesh2"));
	UpButtonMesh2->SetupAttachment(GondolaMesh);

	DownButtonMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DownButtonMesh2"));
	DownButtonMesh2->SetupAttachment(GondolaMesh);

	UpOverlapBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("UpOverlapBox2"));
	UpOverlapBox2->SetupAttachment(UpButtonMesh2);
	UpOverlapBox2->SetBoxExtent(FVector(8.f, 8.f, 8.f));

	DownOverlapBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("DownOverlapBox2"));
	DownOverlapBox2->SetupAttachment(DownButtonMesh2);
	DownOverlapBox2->SetBoxExtent(FVector(8.f, 8.f, 8.f));

	UpOverlapBox2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UpOverlapBox2->SetCollisionResponseToAllChannels(ECR_Ignore);
	UpOverlapBox2->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DownOverlapBox2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DownOverlapBox2->SetCollisionResponseToAllChannels(ECR_Ignore);
	DownOverlapBox2->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	FloorBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorBox"));  // 탑승자 감지 박스 생성
	FloorBox->SetupAttachment(GondolaMesh);                              // 곤돌라 바닥에 부착
	FloorBox->SetBoxExtent(FVector(100.f, 100.f, 50.f));                 // BP에서 실제 바닥 크기에 맞게 조정
	FloorBox->SetCollisionProfileName(TEXT("OverlapAll"));               // 물리 블록 없이 오버랩 이벤트만 발생

	WorkCompleteWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WorkCompleteWidgetComponent"));
	WorkCompleteWidgetComponent->SetupAttachment(GondolaMesh);

	WorkCompleteWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WorkCompleteWidgetComponent->SetDrawSize(FVector2D(1000.f, 700.f));
	WorkCompleteWidgetComponent->SetTwoSided(true);
	WorkCompleteWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	// 곤돌라 기준 고정 위치
	WorkCompleteWidgetComponent->SetRelativeLocation(FVector(0.f, 120.f, 180.f));
	WorkCompleteWidgetComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	WorkCompleteWidgetComponent->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.12f));

	WorkCompleteWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WorkCompleteWidgetComponent->SetCollisionObjectType(ECC_WorldDynamic);
	WorkCompleteWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WorkCompleteWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	WorkCompleteWidgetComponent->SetGenerateOverlapEvents(false);
	WorkCompleteWidgetComponent->SetWindowFocusable(true);
	WorkCompleteWidgetComponent->SetVisibility(false);
}

void AGondola::BeginPlay()
{
	Super::BeginPlay();

	TopWorldZ = GetActorLocation().Z;
	TargetZ = TopWorldZ;
	CurrentFloor = 0;

	UpOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &AGondola::OnUpOverlapBegin);
	DownOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &AGondola::OnDownOverlapBegin);

	if (UpOverlapBox2)
	{
		UpOverlapBox2->OnComponentBeginOverlap.AddDynamic(this, &AGondola::OnUpOverlapBegin2);
	}

	if (DownOverlapBox2)
	{
		DownOverlapBox2->OnComponentBeginOverlap.AddDynamic(this, &AGondola::OnDownOverlapBegin2);
	}

	FloorBox->OnComponentBeginOverlap.AddDynamic(this, &AGondola::OnFloorBeginOverlap);
	FloorBox->OnComponentEndOverlap.AddDynamic(this, &AGondola::OnFloorEndOverlap);

	if (WorkCompleteWidgetComponent && WorkCompleteWidgetClass)
	{
		WorkCompleteWidgetComponent->SetWidgetClass(WorkCompleteWidgetClass);
		WorkCompleteWidgetComponent->InitWidget();
		WorkCompleteWidgetComponent->SetVisibility(false);
	}
}

void AGondola::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;
	if (!bIsMoving) return;

	FVector Loc = GetActorLocation();

	if (FMath::Abs(Loc.Z - TargetZ) > 1.f)
	{
		const float NewZ = FMath::FInterpConstantTo(
			Loc.Z,
			TargetZ,
			DeltaTime,
			MoveSpeed
		);

		const float DeltaZ = NewZ - Loc.Z;

		SetActorLocation(FVector(Loc.X, Loc.Y, NewZ));

		for (AVRPlayerCharacter* Passenger : Passengers)
		{
			if (IsValid(Passenger))
			{
				Passenger->AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ));
			}
		}
	}
	else
	{
		const float DeltaZ = TargetZ - Loc.Z;

		SetActorLocation(FVector(Loc.X, Loc.Y, TargetZ));

		for (AVRPlayerCharacter* Passenger : Passengers)
		{
			if (IsValid(Passenger))
			{
				Passenger->AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ));
			}
		}

		bIsMoving = false;
		SetButtonCollisionEnabled(true);
	}
}

void AGondola::GoUp()
{
	if (!CanOperateButton()) return;
	if (CurrentFloor <= 0) return;

	StartGondolaMoveToFloor(CurrentFloor - 1);
}

void AGondola::GoDown()
{
	if (!CanOperateButton()) return;
	if (CurrentFloor >= NumFloors) return;

	StartGondolaMoveToFloor(CurrentFloor + 1);
}

void AGondola::OnUpOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<USphereComponent>(OtherComp)) return;        // VR 손 구체(SphereComponent)가 아니면 무시
	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);  // 오버랩한 액터가 플레이어인지 확인
	if (Char) Char->PressGondolaButton(this, 1);           // Dir=1 → ServerPressGondolaButton → GoUp
}

void AGondola::OnDownOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<USphereComponent>(OtherComp)) return;        // VR 손 구체가 아니면 무시
	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);
	if (Char) Char->PressGondolaButton(this, -1);          // Dir=-1 → ServerPressGondolaButton → GoDown
}

void AGondola::OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;                                        // 서버에서만 탑승자 목록 관리
	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);   // 플레이어인지 확인
	if (Char) Passengers.AddUnique(Char);                              // 중복 없이 탑승자 목록에 추가
}

// EndOverlap은 BeginOverlap과 달리 bFromSweep, FHitResult 파라미터가 없음
void AGondola::OnFloorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;                                        // 서버에서만 처리
	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);
	if (Char) Passengers.Remove(Char);                                 // 탑승자 목록에서 제거
}

void AGondola::OnUpOverlapBegin2(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<USphereComponent>(OtherComp)) return;

	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);
	if (Char)
	{
		Char->PressGondolaButton(this, 1);
	}
}

void AGondola::OnDownOverlapBegin2(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<USphereComponent>(OtherComp)) return;

	AVRPlayerCharacter* Char = Cast<AVRPlayerCharacter>(OtherActor);
	if (Char)
	{
		Char->PressGondolaButton(this, -1);
	}
}

void AGondola::MulticastShowWorkCompleteWidget_Implementation(float Money, int32 ElapsedSeconds)
{
	if (!WorkCompleteWidgetComponent || !WorkCompleteWidgetClass)
	{
		return;
	}

	if (!WorkCompleteWidgetComponent->GetWidget())
	{
		WorkCompleteWidgetComponent->SetWidgetClass(WorkCompleteWidgetClass);
		WorkCompleteWidgetComponent->InitWidget();
	}

	WorkCompleteWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WorkCompleteWidgetComponent->SetCollisionObjectType(ECC_WorldDynamic);
	WorkCompleteWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WorkCompleteWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	WorkCompleteWidgetComponent->SetWindowFocusable(true);

	WorkCompleteWidgetComponent->SetVisibility(true);

	UUserWidget* UserWidget = WorkCompleteWidgetComponent->GetUserWidgetObject();
	UVR_WorkCompleteWidget* CompleteWidget = Cast<UVR_WorkCompleteWidget>(UserWidget);

	if (CompleteWidget)
	{
		CompleteWidget->UpdateInfo(Money, ElapsedSeconds);
	}
}

bool AGondola::CanOperateButton() const
{
	return !bIsMoving;
}

void AGondola::SetButtonCollisionEnabled(bool bEnabled)
{
	const ECollisionEnabled::Type NewCollision =
		bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;

	if (UpOverlapBox)
	{
		UpOverlapBox->SetCollisionEnabled(NewCollision);
	}

	if (DownOverlapBox)
	{
		DownOverlapBox->SetCollisionEnabled(NewCollision);
	}

	if (UpOverlapBox2)
	{
		UpOverlapBox2->SetCollisionEnabled(NewCollision);
	}

	if (DownOverlapBox2)
	{
		DownOverlapBox2->SetCollisionEnabled(NewCollision);
	}
}

void AGondola::StartGondolaMoveToFloor(int32 NewFloor)
{
	if (!HasAuthority()) return;
	if (!CanOperateButton()) return;

	NewFloor = FMath::Clamp(NewFloor, 0, NumFloors);

	if (NewFloor == CurrentFloor)
	{
		return;
	}

	CurrentFloor = NewFloor;
	TargetZ = TopWorldZ - CurrentFloor * FloorHeight;

	bIsMoving = true;
	SetButtonCollisionEnabled(false);

	MulticastPlayMoveWarningSound();

	if (ASkyGameState* GS = GetWorld()->GetGameState<ASkyGameState>())
	{
		GS->GondolaFloor = CurrentFloor;
	}
}

void AGondola::MulticastPlayMoveWarningSound_Implementation()
{
	if (MoveWarningSound)
	{
		// 곤돌라의 현재 위치에서 소리가 발생하도록 3D 재생
		UGameplayStatics::PlaySoundAtLocation(this, MoveWarningSound, GetActorLocation());
	}
}
