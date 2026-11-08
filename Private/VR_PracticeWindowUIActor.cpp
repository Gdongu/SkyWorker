#include "VR_PracticeWindowUIActor.h"
#include "VR_PracticeWindowWidget.h"
#include "VR_Dirt.h"
#include "VR_WaterResidue.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"

AVR_PracticeWindowUIActor::AVR_PracticeWindowUIActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	WindowAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WindowAreaBox"));
	WindowAreaBox->SetupAttachment(RootScene);
	WindowAreaBox->SetBoxExtent(FVector(50.f, 250.f, 200.f));
	WindowAreaBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WindowAreaBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WindowAreaBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	WindowAreaBox->SetGenerateOverlapEvents(true);

	PracticeWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PracticeWidgetComponent"));
	PracticeWidgetComponent->SetupAttachment(RootScene);
	PracticeWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	PracticeWidgetComponent->SetDrawSize(FVector2D(500.f, 350.f));
	PracticeWidgetComponent->SetTwoSided(true);
	PracticeWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
}

void AVR_PracticeWindowUIActor::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		UpdateTimerHandle,
		this,
		&ThisClass::UpdatePracticeUI,
		UpdateInterval,
		true
	);

	UpdatePracticeUI();
}

void AVR_PracticeWindowUIActor::UpdateTrackedDirtActors()
{
	if (!WindowAreaBox) return;

	TArray<AActor*> OverlappedActors;
	WindowAreaBox->GetOverlappingActors(OverlappedActors, AVR_Dirt::StaticClass());

	for (AActor* Actor : OverlappedActors)
	{
		AVR_Dirt* Dirt = Cast<AVR_Dirt>(Actor);
		if (!Dirt) continue;

		if (!TargetDirtActors.Contains(Dirt))
		{
			TargetDirtActors.Add(Dirt);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					2.f,
					FColor::Green,
					FString::Printf(TEXT("[PracticeUI] Dirt Tracked: %s"), *Dirt->GetName())
				);
			}
		}
	}
	SaveInitialDirtSpawnData();
}

void AVR_PracticeWindowUIActor::UpdateTrackedDirtyWaters()
{
	if (!WindowAreaBox) return;

	const int32 RequiredCount = TargetDirtActors.Num();
	if (RequiredCount <= 0) return;

	TArray<AActor*> OverlappedActors;
	WindowAreaBox->GetOverlappingActors(OverlappedActors, AVR_WaterResidue::StaticClass());

	for (AActor* Actor : OverlappedActors)
	{
		AVR_WaterResidue* Water = Cast<AVR_WaterResidue>(Actor);
		if (!Water) continue;
		if (!Water->bIsDirty) continue;

		if (!TrackedDirtyWaters.Contains(Water))
		{
			TrackedDirtyWaters.Add(Water);
		}
	}
}

void AVR_PracticeWindowUIActor::UpdatePracticeUI()
{
	UpdateTrackedDirtActors();
	UpdateTrackedDirtyWaters();

	const float SprayProgress = CalculateSprayProgress();
	const float WipeProgress = CalculateWipeProgress();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			900,
			0.1f,
			FColor::Cyan,
			FString::Printf(
				TEXT("[PracticeUI] Dirt: %d | Spray: %.2f | Water: %d | Wipe: %.2f"),
				TargetDirtActors.Num(),
				SprayProgress,
				TrackedDirtyWaters.Num(),
				WipeProgress
			)
		);
	}

	if (PracticeWidgetComponent)
	{
		UVR_PracticeWindowWidget* PracticeWidget =
			Cast<UVR_PracticeWindowWidget>(PracticeWidgetComponent->GetUserWidgetObject());

		if (PracticeWidget)
		{
			PracticeWidget->SetPracticeProgress(SprayProgress, WipeProgress);
		}
	}

	// 분사/닦기 둘 다 100%면 5초 뒤 초기화 예약
	ScheduleResetIfCompleted(SprayProgress, WipeProgress);
}

float AVR_PracticeWindowUIActor::CalculateSprayProgress() const
{
	if (TargetDirtActors.Num() <= 0)
	{
		return 0.f;
	}

	float TotalProgress = 0.f;

	for (const TObjectPtr<AVR_Dirt>& Dirt : TargetDirtActors)
	{
		if (!IsValid(Dirt))
		{
			// Destroy된 먼지는 완료 처리
			TotalProgress += 1.f;
			continue;
		}

		TotalProgress += Dirt->GetCleaningProgress();
	}

	return TotalProgress / TargetDirtActors.Num();
}

float AVR_PracticeWindowUIActor::CalculateWipeProgress() const
{
	const int32 RequiredCount = TargetDirtActors.Num();

	if (RequiredCount <= 0)
	{
		return 0.f;
	}

	if (TrackedDirtyWaters.Num() <= 0)
	{
		return 0.f;
	}

	float TotalProgress = 0.f;

	for (const TObjectPtr<AVR_WaterResidue>& Water : TrackedDirtyWaters)
	{
		if (!IsValid(Water))
		{
			// Destroy된 구정물은 닦기 완료로 처리
			TotalProgress += 1.f;
			continue;
		}

		TotalProgress += Water->GetWipingProgress();
	}

	return FMath::Clamp(TotalProgress / RequiredCount, 0.f, 1.f);
}

void AVR_PracticeWindowUIActor::SaveInitialDirtSpawnData()
{
	if (bInitialDirtSaved)
	{
		return;
	}

	if (TargetDirtActors.Num() <= 0)
	{
		return;
	}

	InitialDirtClasses.Empty();
	InitialDirtTransforms.Empty();

	for (const TObjectPtr<AVR_Dirt>& Dirt : TargetDirtActors)
	{
		if (!IsValid(Dirt))
		{
			continue;
		}

		InitialDirtClasses.Add(Dirt->GetClass());
		InitialDirtTransforms.Add(Dirt->GetActorTransform());
	}

	bInitialDirtSaved = InitialDirtClasses.Num() > 0;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			903,
			3.f,
			bInitialDirtSaved ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[PracticeUI] Saved Initial Dirt Count: %d"), InitialDirtClasses.Num())
		);
	}
}

void AVR_PracticeWindowUIActor::ScheduleResetIfCompleted(float SprayProgress, float WipeProgress)
{
	if (bResetScheduled)
	{
		return;
	}

	if (SprayProgress >= 1.f && WipeProgress >= 1.f)
	{
		bResetScheduled = true;

		GetWorldTimerManager().SetTimer(
			ResetPracticeTimerHandle,
			this,
			&ThisClass::ResetPracticeWindow,
			ResetDelay,
			false
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				901,
				ResetDelay,
				FColor::Green,
				FString::Printf(TEXT("[PracticeUI] Complete. Reset after %.1f sec"), ResetDelay)
			);
		}
	}
}

void AVR_PracticeWindowUIActor::ResetPracticeWindow()
{
	GetWorldTimerManager().ClearTimer(ResetPracticeTimerHandle);

	// 현재 박스 안에 남아있는 물자국 전부 제거
	if (WindowAreaBox)
	{
		TArray<AActor*> OverlappedWaters;
		WindowAreaBox->GetOverlappingActors(OverlappedWaters, AVR_WaterResidue::StaticClass());

		for (AActor* Actor : OverlappedWaters)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy();
			}
		}

		// 혹시 남아있는 Dirt도 제거
		TArray<AActor*> OverlappedDirts;
		WindowAreaBox->GetOverlappingActors(OverlappedDirts, AVR_Dirt::StaticClass());

		for (AActor* Actor : OverlappedDirts)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
	}

	TargetDirtActors.Empty();
	TrackedDirtyWaters.Empty();

	UWorld* World = GetWorld();
	if (World)
	{
		for (int32 i = 0; i < InitialDirtClasses.Num(); ++i)
		{
			if (!InitialDirtClasses[i])
			{
				continue;
			}

			const FTransform SpawnTransform = InitialDirtTransforms.IsValidIndex(i)
				? InitialDirtTransforms[i]
				: GetActorTransform();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AVR_Dirt* NewDirt = World->SpawnActor<AVR_Dirt>(
				InitialDirtClasses[i],
				SpawnTransform,
				SpawnParams
			);

			if (NewDirt)
			{
				TargetDirtActors.Add(NewDirt);
			}
		}
	}

	bResetScheduled = false;

	// UI 즉시 0%로 갱신
	if (PracticeWidgetComponent)
	{
		UVR_PracticeWindowWidget* PracticeWidget =
			Cast<UVR_PracticeWindowWidget>(PracticeWidgetComponent->GetUserWidgetObject());

		if (PracticeWidget)
		{
			PracticeWidget->SetPracticeProgress(0.f, 0.f);
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			902,
			2.f,
			FColor::Cyan,
			TEXT("[PracticeUI] Practice window reset")
		);
	}
}