// Fill out your copyright notice in the Description page of Project Settings.
// VRToolBase 상속받은 클래스

#include "VRTool_Cleaner.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "VR_WaterResidue.h"
#include "Net/UnrealNetwork.h"


AVRTool_Cleaner::AVRTool_Cleaner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CleanerBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CleanerBoxCollision"));
	CleanerBoxCollision->SetIsReplicated(false);

	if (RootComponent) {
		CleanerBoxCollision->SetupAttachment(RootComponent);
	}

	// 임시 사이즈
	CleanerBoxCollision->SetBoxExtent(FVector(15.f, 20.f, 15.f));

	// 오버랩 이벤트가 원활하게 발생하도록 프로파일 설정
	CleanerBoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CleanerBoxCollision->SetGenerateOverlapEvents(true);

	CurrentTargetWindow = nullptr;
}


void AVRTool_Cleaner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVRTool_Cleaner, bIsWiping);
}


void AVRTool_Cleaner::BeginPlay()
{
	Super::BeginPlay();

	if (CleanerBoxCollision) {
		CleanerBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AVRTool_Cleaner::OnCleanerBeginOverlap);
		CleanerBoxCollision->OnComponentEndOverlap.AddDynamic(this, &AVRTool_Cleaner::OnCleanerEndOverlap);
	}
}

void AVRTool_Cleaner::OnCleanerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled()) {	// 로컬 플레이어만 청소 연산 시작하도록
		if (OtherActor && OtherActor != this && OtherActor != GetOwner()) {
			if (OtherActor->ActorHasTag(FName("Window"))) {	// 임시로 창문으로 설정, 오염물에 작용시키려면 dirty 등으로 바꿔서 사용
				CurrentTargetWindow = OtherActor;

				LastCleanerLocation = CleanerBoxCollision->GetComponentLocation();

				GetWorldTimerManager().SetTimer(CleaningTimerHandle, this, &AVRTool_Cleaner::ProcessCleaning, 0.1f, true);

				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("[Cleaner] Touch Started"));
			}
		}
	}
}

void AVRTool_Cleaner::OnCleanerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled()) {	// 타이머 관리는 로컬에서만 // 청소계산은 ServerProgressCleaning를 통해 서버에서
		if (OtherActor && OtherActor == CurrentTargetWindow) {
			CurrentTargetWindow = nullptr;
			GetWorldTimerManager().ClearTimer(CleaningTimerHandle);

			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("[Cleaner] Touch Ended"));

			// 사운드 정지 - 창문에서 손을 떼면 무조건 끔
			if (bLocalIsWiping) {
				bLocalIsWiping = false;
				ServerSetWiping(false);
				//K2_OnWipingChanged(false);
			}
		}
	}
}

void AVRTool_Cleaner::ProcessCleaning()
{
	if (CurrentTargetWindow) {
		float CleaningPowerPerTick = 3.f; // 0.1초당 1 -> 1초당 10의 청소수치 증가 // 1너무 오래 걸리는데? 3으로 테스트 함

		FVector CurrentLocation = CleanerBoxCollision->GetComponentLocation();
		float DistanceMoved = FVector::Dist(CurrentLocation, LastCleanerLocation);

		if (DistanceMoved > 2.f) {
			ServerProgressCleaning(CurrentTargetWindow, CleaningPowerPerTick);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("[Cleaner] Working"));

			// 사운드 관련 변수 변경
			if (!bLocalIsWiping) {
				bLocalIsWiping = true;
				ServerSetWiping(true);
			}
		}
		else {
			if (bLocalIsWiping) {
				bLocalIsWiping = false;
				ServerSetWiping(false);
				//K2_OnWipingChanged(false);
			}
		}
		LastCleanerLocation = CurrentLocation;
	}
	else {
		GetWorldTimerManager().ClearTimer(CleaningTimerHandle);
	}
}



void AVRTool_Cleaner::ServerProgressCleaning_Implementation(AActor* TargetWindow, float Amount)
{
	if (HasAuthority() && TargetWindow) {

		TArray<AActor*> OverlappedWaters;
		CleanerBoxCollision->GetOverlappingActors(OverlappedWaters, AVR_WaterResidue::StaticClass());

		for (AActor* Actor : OverlappedWaters) {
			if (AVR_WaterResidue* Water = Cast<AVR_WaterResidue>(Actor)) {
				Water->TakeWipingDamage(Amount);
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(700, 0.5f, FColor::Yellow,
				FString::Printf(TEXT("[Cleaner] Cleaning... Power: %.1f | Target: %s"), Amount, *TargetWindow->GetName()));
		}

	}
}

void AVRTool_Cleaner::OnRep_IsWiping()
{
	K2_OnWipingChanged(bIsWiping);
}

void AVRTool_Cleaner::ServerSetWiping_Implementation(bool bWiping)
{
	bIsWiping = bWiping;

	// 호스트(서버) 플레이어가 직접 닦을 때는 OnRep이 안 불리므로 강제로 한 번 호출
	if (GetNetMode() != NM_DedicatedServer) {
		OnRep_IsWiping();
	}
}

