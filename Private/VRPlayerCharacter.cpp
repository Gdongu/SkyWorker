#include "VRPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/CapsuleComponent.h"
#include "VRInventoryComponent.h"
#include "VRSafetyComponent.h"
#include "VRPlayerState.h"
#include "Gondola.h"
#include "GondolaBoardingStation.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VR_WorkWatch.h"
#include "VR_Bucket.h"
#include "VRTool_Washer.h"
#include "VRGameInstance.h"
#include "Components/WidgetInteractionComponent.h" // 튜토리얼 WBP 버튼 누르기 위해 추가
#include "Blueprint/UserWidget.h"
#include "VR_WindWarningWidget.h"
#include "VR_WorkWatchWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "VR_WorkCompleteWidget.h"
#include "SkyGameState.h"
//[추가] include 게임모드
#include "SkyGameModeBase.h"
#include "VRPlayer_WaterWidget.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"	//사운드

// 테스트용
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"
#include "InputCoreTypes.h"

// Sets default values
AVRPlayerCharacter::AVRPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// VR 기본 하드웨어 설정
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	VROrigin->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);

	// VR용 바람 경고 UI
	WindWarningWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WindWarningWidgetComponent"));
	WindWarningWidgetComponent->SetupAttachment(VRCamera);

	WindWarningWidgetComponent->SetWidgetSpace(EWidgetSpace::World);

	WindWarningWidgetComponent->SetDrawSize(FVector2D(1000.f, 600.f));
	WindWarningWidgetComponent->SetTwoSided(true);
	WindWarningWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	WindWarningWidgetComponent->SetRelativeLocation(FVector(150.f, 0.f, 0.f));
	WindWarningWidgetComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	// 실제 월드 크기 조절
	WindWarningWidgetComponent->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	WindWarningWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WindWarningWidgetComponent->SetVisibility(false);

	LeftHandController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHandController"));
	LeftHandController->SetupAttachment(VROrigin);
	LeftHandController->SetTrackingMotionSource(FName("Left"));

	RightHandController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHandController"));
	RightHandController->SetupAttachment(VROrigin);
	RightHandController->SetTrackingMotionSource(FName("Right"));

	LeftHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandSphere"));
	LeftHandSphere->SetupAttachment(LeftHandController);
	LeftHandSphere->SetSphereRadius(5.f);
	LeftHandSphere->SetCollisionProfileName(TEXT("OverlapAll"));

	RightHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandSphere"));
	RightHandSphere->SetupAttachment(RightHandController);
	RightHandSphere->SetSphereRadius(5.f);
	RightHandSphere->SetCollisionProfileName(TEXT("OverlapAll"));

	// 자기 자신의 메시는 보이지 않게
	if (GetMesh()) {
		GetMesh()->SetOwnerNoSee(true);
	}

	// 1인칭 왼쪽 손 메시
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHandController);
	LeftHandMesh->SetOnlyOwnerSee(true);

	// 1인칭 오른쪽 손 메시
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHandController);
	RightHandMesh->SetOnlyOwnerSee(true);

	// 인벤토리 컴포넌트 생성 및 초기화
	InventoryComponent = CreateDefaultSubobject<UVRInventoryComponent>(TEXT("InventoryComponent"));
	SafetyComponent = CreateDefaultSubobject<UVRSafetyComponent>(TEXT("SafetyComponent"));

	// 위젯 상호작용
	LeftWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("LeftWidgetInteraction"));
	LeftWidgetInteraction->SetupAttachment(LeftHandController);
	LeftWidgetInteraction->InteractionDistance = 500.f;
	LeftWidgetInteraction->bShowDebug = false;
	LeftWidgetInteraction->TraceChannel = ECC_Visibility;

	RightWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("RightWidgetInteraction"));
	RightWidgetInteraction->SetupAttachment(RightHandController);
	RightWidgetInteraction->InteractionDistance = 500.f;
	RightWidgetInteraction->bShowDebug = false;
	RightWidgetInteraction->TraceChannel = ECC_Visibility;

	// 손목 시계 메쉬
	WatchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WatchMesh"));
	WatchMesh->SetupAttachment(LeftHandController);
	WatchMesh->SetVisibility(false, true);
	WatchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 손목 위치 BP에서 조절 가능
	WatchMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	WatchMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	WatchMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	// 시계 유리 부분 WBP
	WatchWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WatchWidget"));
	WatchWidget->SetupAttachment(WatchMesh);
	WatchWidget->SetVisibility(false);
	WatchWidget->SetWidgetSpace(EWidgetSpace::World);
	WatchWidget->SetDrawSize(FVector2D(100.f, 100.f));
	WatchWidget->SetTwoSided(true);

	// 시계 메쉬 유리 위에 오도록 BP에서 조절
	WatchWidget->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
	WatchWidget->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	WatchWidget->SetRelativeScale3D(FVector(0.03f, 0.03f, 0.03f));

	// 작업 완료 UI
	WorkCompleteWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WorkCompleteWidgetComponent"));
	WorkCompleteWidgetComponent->SetupAttachment(VRCamera);

	WorkCompleteWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WorkCompleteWidgetComponent->SetDrawSize(FVector2D(1000.f, 700.f));
	WorkCompleteWidgetComponent->SetTwoSided(true);
	WorkCompleteWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	WorkCompleteWidgetComponent->SetRelativeLocation(FVector(180.f, 0.f, 0.f));
	WorkCompleteWidgetComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	WorkCompleteWidgetComponent->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.12f));

	WorkCompleteWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WorkCompleteWidgetComponent->SetCollisionObjectType(ECC_WorldDynamic);
	WorkCompleteWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WorkCompleteWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	WorkCompleteWidgetComponent->SetGenerateOverlapEvents(false);
	WorkCompleteWidgetComponent->SetWindowFocusable(true);
	WorkCompleteWidgetComponent->SetVisibility(false);

	WaterHitWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WaterHitWidget"));
	WaterHitWidgetComponent->SetupAttachment(VRCamera);
	WaterHitWidgetComponent->SetRelativeLocation(FVector(25.f, 0.f, 0.f));
	WaterHitWidgetComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	WaterHitWidgetComponent->SetRelativeScale3D(FVector(0.05f));
	WaterHitWidgetComponent->SetDrawSize(FVector2D(1000.f, 1000.f));
	WaterHitWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WaterHitWidgetComponent->SetTwoSided(true);
	WaterHitWidgetComponent->SetVisibility(false);
}

// Called when the game starts or when spawned
void AVRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(WatchUpdateTimerHandle, this, &AVRPlayerCharacter::UpdateWatchWidget, 1.f, true);

	if (WindWarningWidgetComponent && WindWarningWidgetClass)
	{
		WindWarningWidgetComponent->SetWidgetClass(WindWarningWidgetClass);
		WindWarningWidgetComponent->InitWidget();
		WindWarningWidgetComponent->SetVisibility(false);
	}

	if (WorkCompleteWidgetComponent && WorkCompleteWidgetClass)
	{
		WorkCompleteWidgetComponent->SetWidgetClass(WorkCompleteWidgetClass);
		WorkCompleteWidgetComponent->InitWidget();
		WorkCompleteWidgetComponent->SetVisibility(false);
	}

	if (GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice() && GEngine->XRSystem->GetHMDDevice()->IsHMDConnected()) {
		// VR모드
		VROrigin->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	}
	else { // PC 키보드 테스트용
		// PC 테스트 모드
		VROrigin->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

		// 마우스로 전체 회전
		bUseControllerRotationYaw = true;
		VRCamera->bUsePawnControlRotation = true;
		if (LeftHandController && RightHandController)
		{
			// 오른쪽 컨트롤러 앞에 있는것처럼 임시 위치 (X=앞, Y=옆, Z=아래)
			RightHandController->SetRelativeLocation(FVector(35.f, 25.f, -20.f));

			// 왼쪽 컨트롤러 앞에 있는것처럼 임시 위치
			LeftHandController->SetRelativeLocation(FVector(35.f, -25.f, -20.f));
		}
	}
}

// Called every frame
void AVRPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (IsLocallyControlled())
	//{
	//	if (LeftHandController && RightHandController)
	//	{
	//		// 만약 내가 호스트(서버)라면 굳이 RPC를 안 부르고 변수에 바로 넣어도 자동 복제됨
	//		if (HasAuthority())
	//		{
	//			ReplicatedLeftHandTransform = LeftHandController->GetRelativeTransform();
	//			ReplicatedRightHandTransform = RightHandController->GetRelativeTransform();
	//		}
	//		else // 일반 클라이언트라면 서버로 내 위치 전송
	//		{
	//			ServerSyncHandTransforms(LeftHandController->GetRelativeTransform(), RightHandController->GetRelativeTransform());
	//		}
	//	}
	//}
	//// 2. 다른 플레이어의 캐릭터를 내 화면에서 볼 때 (데이터를 받아서 적용)
	//else
	//{
	//	if (LeftHandController && RightHandController)
	//	{
	//		LeftHandController->SetRelativeTransform(ReplicatedLeftHandTransform);
	//		RightHandController->SetRelativeTransform(ReplicatedRightHandTransform);
	//	}
	//}

	//[추가, 수정 사항] 손 위치 데이터를 생성해서 서버에 전송, 서버는 이를 복제해서 받음
	//위의 코드가 제대로 작동하지 않는 이유 : UMotionControllerComponent는 로컬 XR 장비 위치를 직접 계속 읽어서 Transform을 갱신하는 컴포넌트라 Tick에서 갱신하려 할 때 다음 Tick에서 덮일 수 있음
	if (!IsLocallyControlled()) return;
	HandReplicationSendTimer += DeltaTime;

	float SendInterval = 1.f / 30.f; //(1초에 30초번 (30HZ))

	if (HandReplicationSendTimer < SendInterval) return;

	HandReplicationSendTimer = 0.f;

	FVRHandData LeftHandData = MakeHandReplicatedData(LeftHandController);
	FVRHandData RightHandData = MakeHandReplicatedData(RightHandController);

	if (HasAuthority()) {
		ReplicatedLeftHand = LeftHandData;
		ReplicatedRightHand = RightHandData;
	}
	else {
		ServerUpdateVRHandData(LeftHandData, RightHandData);
	}

}

void AVRPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	// 기존에 등록된 변수들이 있다면 그 아래에 추가합니다.
	DOREPLIFETIME(AVRPlayerCharacter, ReplicatedLeftHandTransform);
	DOREPLIFETIME(AVRPlayerCharacter, ReplicatedRightHandTransform);

	//[추가 사항] 손 위치/회전 데이터 Rep
	DOREPLIFETIME_CONDITION(AVRPlayerCharacter, ReplicatedLeftHand, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AVRPlayerCharacter, ReplicatedRightHand, COND_SkipOwner);
}

void AVRPlayerCharacter::ServerSyncHandTransforms_Implementation(FTransform LeftTransform, FTransform RightTransform)
{
	ReplicatedLeftHandTransform = LeftTransform;
	ReplicatedRightHandTransform = RightTransform;
}

// Called to bind functionality to input
void AVRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("Status 1: SetupPlayerInputComponent Executed!"));

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, TEXT("Status 2: Cast to EnhancedInputComponent SUCCESS!"));

		if (IA_Move) {
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Status 3: IA_Move is VALID! Binding now..."));

			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVRPlayerCharacter::Move);
		}
		else {
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Status 3: [ERROR] IA_Move is NULL! Asset Link Broken!"));
		}
		if (IA_SwitchLeftTool) 	EnhancedInputComponent->BindAction(IA_SwitchLeftTool, ETriggerEvent::Started, this, &AVRPlayerCharacter::SwitchTool, EHandType::Left);
		if (IA_SwitchRightTool)	EnhancedInputComponent->BindAction(IA_SwitchRightTool, ETriggerEvent::Started, this, &AVRPlayerCharacter::SwitchTool, EHandType::Right);
		if (IA_SwitchLeftToolReverse) 	EnhancedInputComponent->BindAction(IA_SwitchLeftToolReverse, ETriggerEvent::Started, this, &AVRPlayerCharacter::SwitchToolReverse, EHandType::Left);
		if (IA_SwitchRightToolReverse)	EnhancedInputComponent->BindAction(IA_SwitchRightToolReverse, ETriggerEvent::Started, this, &AVRPlayerCharacter::SwitchToolReverse, EHandType::Right);
		if (IA_UseLeftTool) {
			EnhancedInputComponent->BindAction(IA_UseLeftTool, ETriggerEvent::Started, this, &AVRPlayerCharacter::StartUseTool, EHandType::Left);
			EnhancedInputComponent->BindAction(IA_UseLeftTool, ETriggerEvent::Completed, this, &AVRPlayerCharacter::StopUseTool, EHandType::Left);
		}
		if (IA_UseRightTool) {
			EnhancedInputComponent->BindAction(IA_UseRightTool, ETriggerEvent::Started, this, &AVRPlayerCharacter::StartUseTool, EHandType::Right);
			EnhancedInputComponent->BindAction(IA_UseRightTool, ETriggerEvent::Completed, this, &AVRPlayerCharacter::StopUseTool, EHandType::Right);
		}

		if (IA_InteractLeft)
		{
			// WBP 버튼을 보고 있으면 Button OnClicked 실행
			EnhancedInputComponent->BindAction(IA_InteractLeft, ETriggerEvent::Started, this, &AVRPlayerCharacter::PressWidgetButton, EHandType::Left);
			EnhancedInputComponent->BindAction(IA_InteractLeft, ETriggerEvent::Completed, this, &AVRPlayerCharacter::ReleaseWidgetButton, EHandType::Left);

			// 기존 Interact 실행
			EnhancedInputComponent->BindAction(IA_InteractLeft, ETriggerEvent::Started, this, &AVRPlayerCharacter::Interact, EHandType::Left);
		}

		if (IA_InteractRight)
		{
			// WBP 버튼을 보고 있으면 Button OnClicked 실행
			EnhancedInputComponent->BindAction(IA_InteractRight, ETriggerEvent::Started, this, &AVRPlayerCharacter::PressWidgetButton, EHandType::Right);
			EnhancedInputComponent->BindAction(IA_InteractRight, ETriggerEvent::Completed, this, &AVRPlayerCharacter::ReleaseWidgetButton, EHandType::Right);

			// 기존 Interact 실행
			EnhancedInputComponent->BindAction(IA_InteractRight, ETriggerEvent::Started, this, &AVRPlayerCharacter::Interact, EHandType::Right);
		}

		if (IA_Turn) {
			EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &AVRPlayerCharacter::Turn);
		}
		if (IA_LookUp) {
			EnhancedInputComponent->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &AVRPlayerCharacter::LookUp);
		}

		// PC 테스트용 곤돌라 키 바인딩
		if (IA_TestGondolaUp) EnhancedInputComponent->BindAction(IA_TestGondolaUp, ETriggerEvent::Started, this, &AVRPlayerCharacter::TestGondolaUp);
		if (IA_TestGondolaDown) EnhancedInputComponent->BindAction(IA_TestGondolaDown, ETriggerEvent::Started, this, &AVRPlayerCharacter::TestGondolaDown);
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Status 2: [ERROR] Cast FAILED! Check Project Settings->Input Classes!"));
	}
}

// PC 테스트용 임시 함수 Turn
void AVRPlayerCharacter::Turn(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	AddControllerYawInput(AxisValue);
}

void AVRPlayerCharacter::LookUp(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	AddControllerPitchInput(AxisValue);
}

void AVRPlayerCharacter::Move(const FInputActionValue& Value)
{
	// [디버그에 추가된 코드] 함수가 호출되면 화면 우하단에 주황색으로 표시됩니다.
	if (GEngine) GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Orange, TEXT("Move Function CALLED! (Signal Received)"));


	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller && (MovementVector.X != 0.0f || MovementVector.Y != 0.0f)) {

		if (GEngine) GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Green, TEXT("SUCCESS: Moving Inside IF Statement!"));

		const FRotator Rotation = VRCamera->GetComponentRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

	}
}

void AVRPlayerCharacter::SwitchTool(EHandType Hand) {
	if (!HasAuthority()) {
		ServerSwitchTool(Hand); return;
	}
	if (InventoryComponent) InventoryComponent->SwitchToolInternal(Hand, true);
}
void AVRPlayerCharacter::ServerSwitchTool_Implementation(EHandType Hand) {
	SwitchTool(Hand);
}

void AVRPlayerCharacter::SwitchToolReverse(EHandType Hand) {
	if (!HasAuthority()) {
		ServerSwitchToolReverse(Hand); return;
	}
	if (InventoryComponent) InventoryComponent->SwitchToolInternal(Hand, false); // false로 보내서 역방향 호출
}

// Server RPC도 똑같이 추가
void AVRPlayerCharacter::ServerSwitchToolReverse_Implementation(EHandType Hand) {
	SwitchToolReverse(Hand);
}

void AVRPlayerCharacter::StartUseTool(EHandType Hand) {
	if (!HasAuthority()) {
		ServerStartUseTool(Hand); return;
	}
	if (InventoryComponent) InventoryComponent->StartUseToolInternal(Hand);
}
void AVRPlayerCharacter::ServerStartUseTool_Implementation(EHandType Hand) {
	StartUseTool(Hand);
}

void AVRPlayerCharacter::StopUseTool(EHandType Hand) {
	if (!HasAuthority()) {
		ServerStopUseTool(Hand); return;
	}
	if (InventoryComponent) InventoryComponent->StopUseToolInternal(Hand);
}
void AVRPlayerCharacter::ServerStopUseTool_Implementation(EHandType Hand) {
	StopUseTool(Hand);
}

void AVRPlayerCharacter::Interact(EHandType Hand)
{
	if (!HasAuthority())
	{
		ServerInteract(Hand);
		return;
	}

	UMotionControllerComponent* TargetHand =
		(Hand == EHandType::Left) ? LeftHandController : RightHandController;

	if (TargetHand)
	{
		FVector StartLoc = TargetHand->GetComponentLocation();
		FVector EndLoc = StartLoc + TargetHand->GetForwardVector() * 80.f;

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (InventoryComponent)
		{
			if (InventoryComponent->LeftToolActor)
			{
				Params.AddIgnoredActor(InventoryComponent->LeftToolActor);
			}

			if (InventoryComponent->RightToolActor)
			{
				Params.AddIgnoredActor(InventoryComponent->RightToolActor);
			}
		}

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLoc,
			EndLoc,
			ECC_Visibility,
			Params
		);

		if (bHit && HitResult.GetActor())
		{
			/*AVR_WorkWatch* WorkWatch = Cast<AVR_WorkWatch>(HitResult.GetActor());
			if (WorkWatch && !WorkWatch->IsEquipped())
			{
				WorkWatch->EquipToPlayer(this);
				return;
			}*/

			AGondolaBoardingStation* Station =
				Cast<AGondolaBoardingStation>(HitResult.GetActor());

			if (Station)
			{
				Station->TryStartWorkLevel(this);
				return;
			}
		}
	}

	if (InventoryComponent)
	{
		InventoryComponent->InteractInternal(Hand);
	}

	if (SafetyComponent)
	{
		SafetyComponent->InteractInternal(Hand);
	}
}

void AVRPlayerCharacter::ServerInteract_Implementation(EHandType Hand) {
	Interact(Hand);
}

void AVRPlayerCharacter::StartGazeTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(BucketCheckTimerHandle))
	{
		GetWorldTimerManager().SetTimer(BucketCheckTimerHandle, this, &AVRPlayerCharacter::CheckBucketWithGaze, 0.1f, true);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Timer Start"));
	}
}
void AVRPlayerCharacter::StopGazeTimer()
{
	if (GetWorldTimerManager().IsTimerActive(BucketCheckTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(BucketCheckTimerHandle);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Tiemr Stop"));

		// [추가] 시선을 뗐을 때 충전 사운드 강제 종료
		if (InventoryComponent) {
			AVRTool_Washer* ActiveWasher = InventoryComponent->GetActiveWasher();
			if (ActiveWasher && ActiveWasher->bLocalIsRefilling) {
				ActiveWasher->bLocalIsRefilling = false;
				ActiveWasher->ServerSetRefilling(false);
			}
		}
	}
}

void AVRPlayerCharacter::SetWindActive(bool bActive)
{
	bWindActive = bActive;
	if (bActive)
	{
		bDeductedThisWind = false;
		GetWorldTimerManager().SetTimer(WindCheckTimerHandle, this,
			&AVRPlayerCharacter::CheckMovementDuringWind, 0.2f, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(WindCheckTimerHandle);
		bDeductedThisWind = false;
	}
}

void AVRPlayerCharacter::CheckMovementDuringWind()
{
	if (!bWindActive || bDeductedThisWind) return;
	if (GetVelocity().SizeSquared() > 1.0f)	// 수치 수정 필요 10000.0f로 또는 FMath::Square(100.f))
	{										// 빠른 연산을 위해 Size() 함수 대신 SizeSquared()를 사용하므로 숫자가 제곱으로 커짐
		bDeductedThisWind = true;			// Root연산은 무겁고 느림
		ServerWage();
	}
}

void AVRPlayerCharacter::ServerWage_Implementation()
{
	AVRPlayerState* PS = GetPlayerState<AVRPlayerState>();
	if (PS) PS->Wage();
}

void AVRPlayerCharacter::PressGondolaButton(AGondola* Gondola, int32 Dir)
{
	ServerPressGondolaButton(Gondola, Dir);
}

void AVRPlayerCharacter::ServerPressGondolaButton_Implementation(AGondola* Gondola, int32 Dir)
{
	if (!Gondola) return;
	if (Dir == 1) Gondola->GoUp();
	else Gondola->GoDown();
}

void AVRPlayerCharacter::TestGondolaUp()
{
	if (!HasAuthority()) { ServerTestGondolaUp(); return; }
	TArray<AActor*> Gondolas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGondola::StaticClass(), Gondolas);
	AGondola* Nearest = nullptr;
	float MinDist = FLT_MAX;
	for (AActor* A : Gondolas)
	{
		float Dist = FVector::Dist(GetActorLocation(), A->GetActorLocation());
		if (Dist < MinDist) { MinDist = Dist; Nearest = Cast<AGondola>(A); }
	}
	if (Nearest) Nearest->GoUp();
}

void AVRPlayerCharacter::ServerTestGondolaUp_Implementation()
{
	TestGondolaUp();
}

void AVRPlayerCharacter::TestGondolaDown()
{
	if (!HasAuthority()) { ServerTestGondolaDown(); return; }
	TArray<AActor*> Gondolas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGondola::StaticClass(), Gondolas);
	AGondola* Nearest = nullptr;
	float MinDist = FLT_MAX;
	for (AActor* A : Gondolas)
	{
		float Dist = FVector::Dist(GetActorLocation(), A->GetActorLocation());
		if (Dist < MinDist) { MinDist = Dist; Nearest = Cast<AGondola>(A); }
	}
	if (Nearest) Nearest->GoDown();
}

void AVRPlayerCharacter::ServerTestGondolaDown_Implementation()
{
	TestGondolaDown();
}

void AVRPlayerCharacter::ClientHideInteractedItem_Implementation(AActor* ItemToHide)
{
	// 이 함수는 오직 이 캐릭터를 조종하는 사람의 컴퓨터 화면에서만 실행
	if (ItemToHide) {
		ItemToHide->SetActorHiddenInGame(true); // 내 화면에서만 투명화

		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ItemToHide->GetRootComponent())) {
			RootPrim->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 내 레이저 통과
		}
		if (EquipSound)	//습득 사운드, Client함수 내부에 위치하여 본인에게만 들리도록 함
		{
			UGameplayStatics::PlaySound2D(GetWorld(), EquipSound);
		}
	}
}

// 시선을 갖다대서 물 충전
void AVRPlayerCharacter::CheckBucketWithGaze()
{
	if (!InventoryComponent) return;

	AVRTool_Washer* ActiveWasher = InventoryComponent->GetActiveWasher();
	if (!ActiveWasher) return;

	FVector StartLoc = VRCamera->GetComponentLocation();
	FVector ForwardVector = VRCamera->GetForwardVector();
	FVector EndLoc = StartLoc + (ForwardVector * 180.f);	// 임시 1.5m

	FHitResult HitResult;
	FCollisionQueryParams Params;

	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ActiveWasher);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, Params);

	if (bHit && HitResult.GetActor()) {
		AVR_Bucket* HitBucket = Cast<AVR_Bucket>(HitResult.GetActor());
		if (HitBucket)
		{
			float RefillAmountperTick = 2.0f;

			// [조건 1] 물이 100% 미만일 때만 충전 로직 & 사운드 ON
			if (ActiveWasher->CurrentWater < ActiveWasher->MaxWater)
			{
				if (!ActiveWasher->bLocalIsRefilling) {
					ActiveWasher->bLocalIsRefilling = true;
					ActiveWasher->ServerSetRefilling(true); // 사운드 켜기
				}
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						300,
						0.5f,
						FColor::Cyan,
						FString::Printf(
							TEXT("Refilling With Gaze : %.1f / %.1f"),
							ActiveWasher->CurrentWater,
							ActiveWasher->MaxWater
						)
					);
				}

				ServerRefillWasher(ActiveWasher, RefillAmountperTick);
			}
			
			else
			{
				// [조건 2] 물이 100% 꽉 찼다면? 사운드 OFF (UI는 아래 코드 덕분에 계속 보임)
				if (ActiveWasher->bLocalIsRefilling) {
					ActiveWasher->bLocalIsRefilling = false;
					ActiveWasher->ServerSetRefilling(false);
				}
			}

			const float PredictedWater = FMath::Clamp(
				ActiveWasher->CurrentWater + RefillAmountperTick,
				0.f,
				ActiveWasher->MaxWater
			);


			HitBucket->ShowRefillWidget(PredictedWater, ActiveWasher->MaxWater);
		}
		else {
			// [예외] 양동이가 아닌 다른 걸 봤을 때 사운드 OFF
			if (ActiveWasher->bLocalIsRefilling) {
				ActiveWasher->bLocalIsRefilling = false;
				ActiveWasher->ServerSetRefilling(false);
			}
		}
	}
	else {
		// [예외] 허공을 봤을 때 사운드 OFF
		if (ActiveWasher->bLocalIsRefilling) {
			ActiveWasher->bLocalIsRefilling = false;
			ActiveWasher->ServerSetRefilling(false);
		}
	}
}

void AVRPlayerCharacter::ServerRefillWasher_Implementation(AVRTool_Washer* Washer, float Amount)
{
	if (Washer) {
		Washer->RefillWater(Amount);
	}
}

void AVRPlayerCharacter::PressWidgetButton(EHandType Hand)
{
	UWidgetInteractionComponent* TargetWidgetInteraction =
		(Hand == EHandType::Left) ? LeftWidgetInteraction : RightWidgetInteraction;

	if (!TargetWidgetInteraction)
	{
		return;
	}

	TargetWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
}

void AVRPlayerCharacter::ReleaseWidgetButton(EHandType Hand)
{
	UWidgetInteractionComponent* TargetWidgetInteraction =
		(Hand == EHandType::Left) ? LeftWidgetInteraction : RightWidgetInteraction;

	if (!TargetWidgetInteraction)
	{
		return;
	}

	TargetWidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AVRPlayerCharacter::ClientSetWindWarningVisible_Implementation(bool bVisible)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!WindWarningWidgetComponent)
	{
		return;
	}

	if (WindWarningWidgetClass && !WindWarningWidgetComponent->GetWidget())
	{
		WindWarningWidgetComponent->SetWidgetClass(WindWarningWidgetClass);
		WindWarningWidgetComponent->InitWidget();
	}

	WindWarningWidgetComponent->SetVisibility(bVisible);
}

void AVRPlayerCharacter::ClientUpdateWindWarningCountdown_Implementation(int32 Count)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!WindWarningWidgetComponent)
	{
		return;
	}

	UUserWidget* UserWidget = WindWarningWidgetComponent->GetUserWidgetObject();
	if (!UserWidget)
	{
		if (WindWarningWidgetClass)
		{
			WindWarningWidgetComponent->SetWidgetClass(WindWarningWidgetClass);
			WindWarningWidgetComponent->InitWidget();
			UserWidget = WindWarningWidgetComponent->GetUserWidgetObject();
		}
	}

	UVR_WindWarningWidget* WarningWidget =
		Cast<UVR_WindWarningWidget>(UserWidget);

	if (!WarningWidget)
	{
		return;
	}

	WarningWidget->SetCountdownNumber(Count);
}

void AVRPlayerCharacter::UpdateWatchWidget()
{
	if (!WatchWidget) return;
	UVR_WorkWatchWidget* WorkWatchWidget = Cast<UVR_WorkWatchWidget>(WatchWidget->GetUserWidgetObject());
	if (!WorkWatchWidget) return;
	ASkyGameState* GS = GetWorld()->GetGameState<ASkyGameState>();
	float TotalProgress = GS ? (GS->SprayProgress * 0.5f + GS->WipeProgress * 0.5f) : 0.f;
	float SyncedElapsedTime = GS ? static_cast<float>(GS->ElapsedSeconds) : 0.f;
	int32 ZoneNum = GS ? (5 - GS->GondolaFloor) : 5;  // NumFloors=5 기준
	FString ZoneName = FString::Printf(TEXT("A%d"), ZoneNum);
	WorkWatchWidget->UpdateWorkInfo(TotalProgress, ZoneName, SyncedElapsedTime);
	WatchWidget->RequestRedraw();
}


void AVRPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UVRGameInstance* GI = GetGameInstance<UVRGameInstance>();
	if (!GI || GI->bSafetyPenaltyApplied) return;

	AVRPlayerState* PS = GetPlayerState<AVRPlayerState>();
	if (!PS) return;

	if (!GI->bHasHelmet)  PS->ApplyPenalty(100000.f);
	if (!GI->bHasHarness) PS->ApplyPenalty(100000.f);
	if (!GI->bHasHook)    PS->ApplyPenalty(100000.f);

	GI->bSafetyPenaltyApplied = true;
}

void AVRPlayerCharacter::SetWatchVisible(bool bVisible)
{
	if (WatchMesh)
	{
		WatchMesh->SetVisibility(bVisible, true);
	}

	if (WatchWidget)
	{
		WatchWidget->SetVisibility(bVisible);
	}
}

void AVRPlayerCharacter::ClientShowWorkCompleteWidget_Implementation()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!WorkCompleteWidgetComponent || !WorkCompleteWidgetClass)
	{
		return;
	}

	if (!WorkCompleteWidgetComponent->GetWidget())
	{
		WorkCompleteWidgetComponent->SetWidgetClass(WorkCompleteWidgetClass);
		WorkCompleteWidgetComponent->InitWidget();
	}

	WorkCompleteWidgetComponent->SetVisibility(true);

	UUserWidget* UserWidget = WorkCompleteWidgetComponent->GetUserWidgetObject();
	UVR_WorkCompleteWidget* CompleteWidget = Cast<UVR_WorkCompleteWidget>(UserWidget);

	AVRPlayerState* PS = GetPlayerState<AVRPlayerState>();
	ASkyGameState* GS = GetWorld()->GetGameState<ASkyGameState>();

	if (CompleteWidget && PS && GS)
	{
		CompleteWidget->UpdateInfo(PS->Money, GS->ElapsedSeconds);
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

void AVRPlayerCharacter::Client_StartWaterHitImage_Implementation() {
	if (!IsLocallyControlled()) return;
	if (!WaterHitWidgetComponent) return;

	WaterHitWidgetComponent->SetVisibility(true);
	WaterHitWidgetComponent->SetHiddenInGame(false);

	UUserWidget* UserWidget = WaterHitWidgetComponent->GetUserWidgetObject();
	ActiveWaterHitWidget = Cast<UVRPlayer_WaterWidget>(UserWidget);


	if (ActiveWaterHitWidget) {
		ActiveWaterHitWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ActiveWaterHitWidget->SetRenderOpacity(1.f);
		ActiveWaterHitWidget->PlayWaterHitEffect();
		GetWorldTimerManager().ClearTimer(WaterHitEffectTimerHandle);
		GetWorldTimerManager().SetTimer(WaterHitEffectTimerHandle, this, &AVRPlayerCharacter::UpdateWaterHitEffectByTimer, 0.016f, true);
	}
}

void AVRPlayerCharacter::UpdateWaterHitEffectByTimer()
{
	if (!ActiveWaterHitWidget) {
		GetWorldTimerManager().ClearTimer(WaterHitEffectTimerHandle);
		return;
	}

	float DeltaTime = 0.016f;
	bool bFinished = ActiveWaterHitWidget->UpdateWaterHitEffect(DeltaTime);

	if (WaterHitWidgetComponent) {
		WaterHitWidgetComponent->RequestRedraw();
	}
	if (bFinished) {
		GetWorldTimerManager().ClearTimer(WaterHitEffectTimerHandle);

		if (WaterHitWidgetComponent) {
			WaterHitWidgetComponent->SetVisibility(false, true);
		}
	}
}

//----------------------------------------------------------
//[추가 사항]

void AVRPlayerCharacter::ServerUpdateVRHandData_Implementation(const FVRHandData& LeftHandData, const FVRHandData& RightHandData)
{
	ReplicatedLeftHand = LeftHandData;
	ReplicatedRightHand = RightHandData;

	//Listen 서버 화면에서 Client의 손 위치가 보이도록 적용
	ApplyHandReplicatedData(LeftHandController, ReplicatedLeftHand);
	ApplyHandReplicatedData(RightHandController, ReplicatedRightHand);
}

void AVRPlayerCharacter::OnRep_VRHands()
{
	ApplyHandReplicatedData(LeftHandController, ReplicatedLeftHand);
	ApplyHandReplicatedData(RightHandController, ReplicatedRightHand);
}

FVRHandData AVRPlayerCharacter::MakeHandReplicatedData(UMotionControllerComponent* HandComp)
{
	FVRHandData Data;
	if (!HandComp) return Data;

	FTransform ActorTransform = GetActorTransform();

	Data.HandLocation = ActorTransform.InverseTransformPosition(HandComp->GetComponentLocation());
	Data.HandRotation = ActorTransform.InverseTransformRotation(HandComp->GetComponentQuat()).Rotator();

	return Data;
}

void AVRPlayerCharacter::ApplyHandReplicatedData(UMotionControllerComponent* HandComp, const FVRHandData& Data)
{
	if (!HandComp) return;

	if (IsLocallyControlled()) return;

	FTransform ActorTransform = GetActorTransform();
	FVector WorldLocation = ActorTransform.TransformPosition(Data.HandLocation);
	FQuat WorldRotation = ActorTransform.TransformRotation(Data.HandRotation.Quaternion());

	HandComp->SetWorldLocationAndRotation(WorldLocation, WorldRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AVRPlayerCharacter::RequestReturnAllPlayersToTitle()
{
	if (HasAuthority()) {
		ServerRequestReturnAllPlayersToTitle_Implementation();
	}
	else {
		ServerRequestReturnAllPlayersToTitle();
	}
}

void AVRPlayerCharacter::LocalReturnToTitle()
{
	UGameplayStatics::OpenLevel(this, FName("/Game/VRTemplate/Maps/VRTemplateMap"));
}

void AVRPlayerCharacter::ClientReturnToTitle_Implementation()
{
	LocalReturnToTitle();
}

void AVRPlayerCharacter::ServerRequestReturnAllPlayersToTitle_Implementation()
{
	if (!HasAuthority()) return;

	AGameModeBase* GM = GetWorld()->GetAuthGameMode();
	if (!GM) return;

	ASkyGameModeBase* SGM = Cast<ASkyGameModeBase>(GM);
	if (!SGM) return;

	SGM->ReturnAllPlayersToTitle();
}