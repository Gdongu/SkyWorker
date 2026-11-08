// Fill out your copyright notice in the Description page of Project Settings.
// 캐릭터 상속받은 클래스

#include "VR_TitlePlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "VRInventoryComponent.h"
#include "VRSafetyComponent.h"
#include "VRTool_Washer.h"
#include "VR_Bucket.h"
#include "InputCoreTypes.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/WidgetComponent.h"	//[추가]

// 테스트용
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"

// Sets default values
AVR_TitlePlayerCharacter::AVR_TitlePlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// VR 기본 하드웨어 세팅
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	VROrigin->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);

	LeftHandController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHandController"));
	LeftHandController->SetupAttachment(VROrigin);
	LeftHandController->SetTrackingMotionSource(FName("Left"));

	RightHandController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHandController"));
	RightHandController->SetupAttachment(VROrigin);
	RightHandController->SetTrackingMotionSource(FName("Right"));

	// 내 눈에는 전신 메쉬가 보이지 않게
	if (GetMesh()) {
		GetMesh()->SetOwnerNoSee(true);
	}

	// 1인칭 전용 왼손 메쉬
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHandController);
	LeftHandMesh->SetOnlyOwnerSee(true);

	// 1인칭 전용 오른손 메쉬
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHandController);
	RightHandMesh->SetOnlyOwnerSee(true);

	// 분리해놓은 컴포넌트 생성 및 장착
	InventoryComponent = CreateDefaultSubobject<UVRInventoryComponent>(TEXT("InventoryComponent"));
	SafetyComponent = CreateDefaultSubobject<UVRSafetyComponent>(TEXT("SafetyComponent"));

	RightWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("RightWidgetInteraction"));
	RightWidgetInteraction->SetupAttachment(RightHandController);
	RightWidgetInteraction->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	LeftWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("LeftWidgetInteraction"));
	LeftWidgetInteraction->SetupAttachment(LeftHandController);
	LeftWidgetInteraction->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	RightWidgetInteraction->InteractionDistance = 500.f;
	RightWidgetInteraction->VirtualUserIndex = 0;
	RightWidgetInteraction->PointerIndex = 0;
	RightWidgetInteraction->bShowDebug = true; // 테스트 끝나면 false
	RightWidgetInteraction->TraceChannel = ECC_Visibility;

	LeftWidgetInteraction->InteractionDistance = 500.f;
	LeftWidgetInteraction->VirtualUserIndex = 0;
	LeftWidgetInteraction->PointerIndex = 1;
	LeftWidgetInteraction->bShowDebug = true; // 테스트 끝나면 false
	LeftWidgetInteraction->TraceChannel = ECC_Visibility;

}



// Called when the game starts or when spawned
void AVR_TitlePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice() && GEngine->XRSystem->GetHMDDevice()->IsHMDConnected()) {
		// VR모드
		VROrigin->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	}
	else { // PC 키보드 테스트용
		// PC 테스트 모드
		VROrigin->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

		// 마우스로 본체 회전
		bUseControllerRotationYaw = true;

		// [추가] 상하 움직임 허용
		VRCamera->bUsePawnControlRotation = true;

		if (LeftHandController && RightHandController)
		{
			// 오른손 컨트롤러를 가슴 앞 오른쪽으로 임시 배치 (X=앞, Y=우, Z=상하)
			RightHandController->SetRelativeLocation(FVector(35.f, 25.f, -20.f));

			// 왼손 컨트롤러를 가슴 앞 왼쪽으로 임시 배치
			LeftHandController->SetRelativeLocation(FVector(35.f, -25.f, -20.f));
		}
	}

	SetupInputMappingContext();
}

// Called every frame
void AVR_TitlePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Called to bind functionality to input
void AVR_TitlePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		if (IA_Move) {
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVR_TitlePlayerCharacter::Move);
		}

		// 상호작용 (UI 상호작용)
		if (IA_InteractLeft) {
			EnhancedInputComponent->BindAction(IA_InteractLeft, ETriggerEvent::Started, this, &AVR_TitlePlayerCharacter::PressUI, EHandType::Left);
			EnhancedInputComponent->BindAction(IA_InteractLeft, ETriggerEvent::Completed, this, &AVR_TitlePlayerCharacter::ReleaseUI, EHandType::Left);
		}
		if (IA_InteractRight) {
			EnhancedInputComponent->BindAction(IA_InteractRight, ETriggerEvent::Started, this, &AVR_TitlePlayerCharacter::PressUI, EHandType::Right);
			EnhancedInputComponent->BindAction(IA_InteractRight, ETriggerEvent::Completed, this, &AVR_TitlePlayerCharacter::ReleaseUI, EHandType::Right);
		}

		if (IA_Turn) {
			EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &AVR_TitlePlayerCharacter::Turn);
		}
	}

}

void AVR_TitlePlayerCharacter::SetupInputMappingContext()
{
	// 현재 화면을 지배하는 로컬 컨트롤러 일 때만 실행
	if (IMC_TitlePlayer) {
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC) {
			ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
			if (LocalPlayer) {
				UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
				if (Subsystem) {
					// 중복 방지를 위해 제거 후 재등록
					Subsystem->RemoveMappingContext(IMC_TitlePlayer);
					Subsystem->AddMappingContext(IMC_TitlePlayer, 0);

					FInputModeGameOnly InputMode;
					PC->SetInputMode(InputMode);
					PC->bShowMouseCursor = false;

				}
			}
		}
	}
}

// PC 테스트용 임시  함수 Turn
void AVR_TitlePlayerCharacter::Turn(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller) {
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVR_TitlePlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && (MovementVector.X != 0.0f || MovementVector.Y != 0.0f)) {

		const FRotator Rotation = VRCamera->GetComponentRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		UE_LOG(LogTemp, Log, TEXT("Move Input: X=%.2f, Y=%.2f"), MovementVector.X, MovementVector.Y);

	}
}

void AVR_TitlePlayerCharacter::PressUI(EHandType Hand)
{
	UWidgetInteractionComponent* TargetInteraction = nullptr;
	switch (Hand) {
	case EHandType::Left:
		TargetInteraction = LeftWidgetInteraction;
		break;
	case EHandType::Right:
		TargetInteraction = RightWidgetInteraction;
		break;
	default:
		break;
	}

	if (!TargetInteraction) return;

	TargetInteraction->PressPointerKey(EKeys::LeftMouseButton);
}

void AVR_TitlePlayerCharacter::ReleaseUI(EHandType Hand) {
	UWidgetInteractionComponent* TargetInteraction = nullptr;
	switch (Hand) {
	case EHandType::Left:
		TargetInteraction = LeftWidgetInteraction;
		break;
	case EHandType::Right:
		TargetInteraction = RightWidgetInteraction;
		break;
	default:
		break;
	}

	if (!TargetInteraction) return;

	TargetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}


