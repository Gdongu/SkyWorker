// Fill out your copyright notice in the Description page of Project Settings.
// VR 인벤토리 컴포넌트 클래스

#include "VRInventoryComponent.h"
#include "Net/UnrealNetwork.h"	// 멀티플레이어 DOREPLIFETIME
#include "MotionControllerComponent.h" // 모션 컨트롤러 컴포넌트 참조
#include "VRPlayerCharacter.h"
#include "VRTool_Washer.h"
#include "VRTool_Cleaner.h"
#include "Kismet/GameplayStatics.h"
#include "VRGameInstance.h"
#include "VR_WorkWatch.h"
#include "VRSafetyComponent.h"

// Sets default values for this component's properties
UVRInventoryComponent::UVRInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	// 프로젝트가 멀티플레이에서 초기화되도록 지정하는 명령어
	SetIsReplicatedByDefault(true);

	CurrentLeftTool = EToolType::None;
	CurrentRightTool = EToolType::None;
	LeftToolActor = nullptr;
	RightToolActor = nullptr;

	StoredWasherWater = 100.0f;
}

void UVRInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRInventoryComponent, CurrentLeftTool);
	DOREPLIFETIME(UVRInventoryComponent, CurrentRightTool);
	DOREPLIFETIME(UVRInventoryComponent, LeftToolActor);
	DOREPLIFETIME(UVRInventoryComponent, RightToolActor);
}



// Called when the game starts
void UVRInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner() && GetOwner()->HasAuthority())
		LoadFromGameInstance();
}

void UVRInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (GetOwner() && GetOwner()->HasAuthority())
		SaveToGameInstance();
}

void UVRInventoryComponent::SaveToGameInstance()
{
	UVRGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UVRGameInstance>() : nullptr;
	if (!GI) { UE_LOG(LogTemp, Warning, TEXT("[Save] GameInstance null")); return; }

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());

	GI->bIsWasherUnlocked = bIsWasherUnlocked;
	GI->bIsCleanerUnlocked = bIsCleanerUnlocked;
	GI->SavedWasherWater = StoredWasherWater;
	GI->bIsWatchEquipped = false;

	if (Player && Player->SafetyComponent)
	{
		GI->bIsWatchEquipped = Player->SafetyComponent->bHasWatch;
	}
}

void UVRInventoryComponent::LoadFromGameInstance()
{
	UVRGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UVRGameInstance>() : nullptr;
	if (!GI) { UE_LOG(LogTemp, Warning, TEXT("[Load] GameInstance null")); return; }

	bIsWasherUnlocked = GI->bIsWasherUnlocked;
	bIsCleanerUnlocked = GI->bIsCleanerUnlocked;
	StoredWasherWater = GI->SavedWasherWater;
	UE_LOG(LogTemp, Warning, TEXT("[Load] Washer:%d Cleaner:%d Watch:%d"), bIsWasherUnlocked, bIsCleanerUnlocked, GI->bIsWatchEquipped);

	if (bIsWasherUnlocked)
		EquipTool(EToolType::Washer, EHandType::Right);
	else if (bIsCleanerUnlocked)
		EquipTool(EToolType::Cleaner, EHandType::Right);

	if (GI->bIsWatchEquipped)
	{
		AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());
		if (Player)
		{
			if (Player->SafetyComponent)
			{
				Player->SafetyComponent->bHasWatch = true;
			}

			Player->SetWatchVisible(true);
		}
	}
}

// Called every frame
void UVRInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 잠금해제 시 bool 값 설정 및 switch 처리
void UVRInventoryComponent::UnlockTool(EToolType ToolToUnLock)
{
	if (GetOwner() && !GetOwner()->HasAuthority()) return;

	if (ToolToUnLock == EToolType::Washer) {
		bIsWasherUnlocked = true;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[Inventory] Washer Unlocked!"));
	}
	if (ToolToUnLock == EToolType::Cleaner) {
		bIsCleanerUnlocked = true;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[Inventory] Cleaner Unlocked!"));
	}
}

// 상호작용. 튜토리얼에서 호출 결과
void UVRInventoryComponent::InteractInternal(EHandType Hand)
{
	if (GetOwner() && !GetOwner()->HasAuthority()) return;

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());
	if (!Player) return;

	UMotionControllerComponent* TargetHand = (Hand == EHandType::Left) ? Player->LeftHandController : Player->RightHandController;

	if (TargetHand) {
		FVector StartLoc = TargetHand->GetComponentLocation();
		FVector ForwardVector = TargetHand->GetForwardVector();
		FVector EndLoc = StartLoc + (ForwardVector * 50.f);	// 앞에서 0.5m 앞

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);
		if (LeftToolActor) Params.AddIgnoredActor(LeftToolActor);
		if (RightToolActor) Params.AddIgnoredActor(RightToolActor);

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, Params);

		if (bHit && HitResult.GetActor()) {
			AActor* HitActor = HitResult.GetActor();
			bool bIsToolItem = false;

			if (HitActor->IsA(AVRTool_Washer::StaticClass())) {
				// 세탁기 습득
				if (!bIsWasherUnlocked) {
					UnlockTool(EToolType::Washer);
					// 세탁기를 바로 장착
					EquipTool(EToolType::Washer, Hand);
					// 세탁기는 최초 물 풀로 설정
					StoredWasherWater = 100.f;
					bIsToolItem = true;
				}
			}
			else if (HitActor->IsA(AVRTool_Cleaner::StaticClass())) {
				if (!bIsCleanerUnlocked) {
					// 청소기 습득
					UnlockTool(EToolType::Cleaner);
					// 청소기를 바로 장착
					EquipTool(EToolType::Cleaner, Hand);
					bIsToolItem = true;
				}
			}
			if (bIsToolItem) {
				// 세탁기나 청소기를 주웠을 때, 해당 플레이어 화면에서만 안보이도록 설정!
				Player->ClientHideInteractedItem(HitActor);
			}
		}

	}
}



void UVRInventoryComponent::SwitchToolInternal(EHandType Hand, bool bIsForward)
{
	EToolType TargetTool = (Hand == EHandType::Left) ? CurrentLeftTool : CurrentRightTool;
	int32 CurrentInt = static_cast<int32>(TargetTool);
	int32 NextInt = 0;

	if (bIsForward) { //앞으로 갈 때 바꿔
		NextInt = (CurrentInt + 1) % 3;
	}
	else {	//뒤로 갈 때 바꿔
		NextInt = (CurrentInt + (3 - 1)) % 3;
	}

	EquipTool(static_cast<EToolType>(NextInt), Hand);
}

void UVRInventoryComponent::EquipTool(EToolType NewToolType, EHandType Hand)
{
	if (!GetOwner()->HasAuthority()) return;

	if (Hand == EHandType::Left) {
		if (LeftToolActor) {
			if (AVRTool_Washer* LeftWasher = Cast<AVRTool_Washer>(LeftToolActor)) {
				StoredWasherWater = LeftWasher->CurrentWater;	// 물 양 저장
			}

			LeftToolActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			LeftToolActor->Destroy();
			LeftToolActor = nullptr;
		}
		CurrentLeftTool = NewToolType;
	}
	else {

		if (RightToolActor) {
			if (AVRTool_Washer* RightWasher = Cast<AVRTool_Washer>(RightToolActor)) {
				StoredWasherWater = RightWasher->CurrentWater;	// 물 양 저장
			}
			RightToolActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			RightToolActor->Destroy();
			RightToolActor = nullptr;

		}
		CurrentRightTool = NewToolType;
	}


	// 없음 상태가 되면 알림
	if (NewToolType == EToolType::None) {
		HandleToolChangeNotification();
		return;
	}

	// 세탁기 잠금해제 안 됐으면? -> 없음 상태로 변경 (도구 없음 표시 용!)
	if (NewToolType == EToolType::Washer && !bIsWasherUnlocked) {
		HandleToolChangeNotification();
		return;
	}

	// 청소기 잠금해제 안 됐으면? -> 없음 상태로 변경 (도구 없음 표시 용!)
	if (NewToolType == EToolType::Cleaner && !bIsCleanerUnlocked) {
		HandleToolChangeNotification();
		return;
	}


	TSubclassOf<AVRToolBase> ClassToSpawn = nullptr;
	if (NewToolType == EToolType::Washer) {
		ClassToSpawn = WasherClass;
	}
	else if (NewToolType == EToolType::Cleaner) {
		ClassToSpawn = CleanerClass;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 월드에 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());
	if (Player) {
		// 해당 손에 스폰 할 모션 컨트롤러 타입 변수
		UMotionControllerComponent* TargetHandComponent = (Hand == EHandType::Left) ? Player->LeftHandController : Player->RightHandController;

		if (TargetHandComponent) {
			FVector SpawnLocation = TargetHandComponent->GetComponentLocation();
			FRotator SpawnRotation = TargetHandComponent->GetComponentRotation();

			AVRToolBase* NewToolActor = World->SpawnActor<AVRToolBase>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

			if (NewToolActor) {
				NewToolActor->AttachToComponent(TargetHandComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

				// 생성된 도구를 인식해서 할당함
				if (Hand == EHandType::Left) {
					LeftToolActor = NewToolActor;
				}
				else {
					RightToolActor = NewToolActor;
				}
				// 새로 생성된 washer에 저장된 물 적용해서 생성
				if (AVRTool_Washer* SpawnedWasher = Cast<AVRTool_Washer>(NewToolActor)) {
					SpawnedWasher->CurrentWater = StoredWasherWater;
				}
				HandleToolChangeNotification();
			}
		}
	}
}



void UVRInventoryComponent::StartUseToolInternal(EHandType Hand)
{
	AVRToolBase* TargetActor = (Hand == EHandType::Left) ? LeftToolActor : RightToolActor;
	if (TargetActor) {
		TargetActor->StartUseTool();
	}
}

void UVRInventoryComponent::StopUseToolInternal(EHandType Hand)
{
	AVRToolBase* TargetActor = (Hand == EHandType::Left) ? LeftToolActor : RightToolActor;
	if (TargetActor) {
		TargetActor->StopUseTool();
	}
}

AVRTool_Washer* UVRInventoryComponent::GetActiveWasher() const
{
	// 왼손이나 오른손에 도구가 있다면 캐스팅해서 반환
	if (class AVRTool_Washer* LeftWasher = Cast<AVRTool_Washer>(LeftToolActor))return LeftWasher;
	if (class AVRTool_Washer* RightWasher = Cast<AVRTool_Washer>(RightToolActor))return RightWasher;

	return nullptr;
}


void UVRInventoryComponent::OnRep_CurrentLeftTool()
{
	HandleToolChangeNotification();
}

void UVRInventoryComponent::OnRep_CurrentRightTool()
{
	HandleToolChangeNotification();
}

void UVRInventoryComponent::HandleToolChangeNotification()
{
	AVRPlayerCharacter* OwnerChar = Cast<AVRPlayerCharacter>(GetOwner());
	if (OwnerChar && OwnerChar->IsLocallyControlled()) {
		if (CurrentLeftTool == EToolType::Washer || CurrentRightTool == EToolType::Washer) {
			OwnerChar->StartGazeTimer();
		}
		else {
			OwnerChar->StopGazeTimer();
		}
	}
}