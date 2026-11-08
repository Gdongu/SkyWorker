// Fill out your copyright notice in the Description page of Project Settings.
// 액터컴포넌트 상속받은 클래스

#include "VRSafetyComponent.h"
#include "Net/UnrealNetwork.h"
#include "VRTool_Washer.h"
#include "VR_Bucket.h"
// [추가]
#include "VRPlayerCharacter.h"
#include "VRInventoryComponent.h"
#include "MotionControllerComponent.h"
#include "VRSafety_Harness.h"
#include "VRSafety_Helmet.h"
#include "VRSafety_Hook.h"
#include "Components/WidgetComponent.h"
#include "VR_WorkWatch.h"
#include "VRGameInstance.h"

// Sets default values for this component's properties
UVRSafetyComponent::UVRSafetyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	// 액터컴포넌트가 멀티플레이에서 복제되도록 설정하는 명령어
	SetIsReplicatedByDefault(true);

	bHasHelmet = false;
	bHasHarness = false;
	bHasHook = false;
	bHasWatch = false;
}

void UVRSafetyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRSafetyComponent, bHasHelmet);
	DOREPLIFETIME(UVRSafetyComponent, bHasHarness);
	DOREPLIFETIME(UVRSafetyComponent, bHasHook);
	DOREPLIFETIME(UVRSafetyComponent, bHasWatch);
}


// Called when the game starts
void UVRSafetyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UVRSafetyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UVRSafetyComponent::InteractInternal(EHandType Hand)
{
	if (!GetOwner()->HasAuthority()) return;

	UE_LOG(LogTemp, Log, TEXT("SafetyComp : %s Hand Does Interaction"), (Hand == EHandType::Left) ? TEXT("Left") : TEXT("Right"));

	// [추가]
	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());
	if (!Player) return;

	UMotionControllerComponent* TargetHand = (Hand == EHandType::Left) ? Player->LeftHandController : Player->RightHandController;

	if (TargetHand) {
		FVector StartLoc = TargetHand->GetComponentLocation();
		FVector ForwardVector = TargetHand->GetForwardVector();
		FVector EndLoc = StartLoc + (ForwardVector * 100.f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);
		if (Player->InventoryComponent) {
			if (Player->InventoryComponent->LeftToolActor) {
				Params.AddIgnoredActor(Player->InventoryComponent->LeftToolActor);
			}
			if (Player->InventoryComponent->RightToolActor) {
				Params.AddIgnoredActor(Player->InventoryComponent->RightToolActor);
			}
		}

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, Params);

		if (bHit && HitResult.GetActor()) {
			AActor* HitActor = HitResult.GetActor();
			bool bIsSafetyItem = false;

			// 워치
			if (HitActor->IsA(AVR_WorkWatch::StaticClass()))
			{
				if (!bHasWatch)
				{
					bHasWatch = true;
					bIsSafetyItem = true;

					Player->SetWatchVisible(true);
				}
			}

			if (HitActor->IsA(AVRSafety_Helmet::StaticClass())) {
				if (!bHasHelmet) {
					bHasHelmet = true;
					bIsSafetyItem = true;
				}
			}
			// 안전대
			if (HitActor->IsA(AVRSafety_Harness::StaticClass())) {
				if (!bHasHarness) {
					bHasHarness = true;
					bIsSafetyItem = true;
				}
			}
			// 안전고리
			if (HitActor->IsA(AVRSafety_Hook::StaticClass())) {
				if (!bHasHook) {
					bHasHook = true;
					bIsSafetyItem = true;
				}
			}

			// [추가]
			if (bIsSafetyItem) {
				// 서버에서 true로, 클라이언트에서 시각적 숨김 처리
				Player->ClientHideInteractedItem(HitActor);
			}
		}
	}
}

void UVRSafetyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	UVRGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UVRGameInstance>() : nullptr;
	if (!GI) return;

	GI->bHasHelmet = bHasHelmet;
	GI->bHasHarness = bHasHarness;
	GI->bHasHook = bHasHook;
}

void UVRSafetyComponent::OnRep_HasWatch()
{
	AVRPlayerCharacter* Player = Cast<AVRPlayerCharacter>(GetOwner());
	if (!Player) return;

	Player->SetWatchVisible(bHasWatch);
}
