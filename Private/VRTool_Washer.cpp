// Fill out your copyright notice in the Description page of Project Settings.
// VRToolBase 상속받은 클래스

#include "VRTool_Washer.h"
#include "Net/UnrealNetwork.h"
#include "VR_Dirt.h"
#include "VR_WaterResidue.h"
#include "VRPlayerCharacter.h"
// 디버그드로우
#include "DrawDebugHelpers.h"

AVRTool_Washer::AVRTool_Washer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ToolType = EToolType::Washer;

	MaxWater = 100.0f;
	CurrentWater = MaxWater;
	WaterConsumptionRate = 3.0f;	// 초당 소모
	bIsSpraying = false;
	bLocalIsRefilling = false;
}

void AVRTool_Washer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVRTool_Washer, CurrentWater);
	DOREPLIFETIME(AVRTool_Washer, bIsSpraying);
	DOREPLIFETIME(AVRTool_Washer, bIsRefilling);
}

void AVRTool_Washer::StartUseTool()
{
	if (CurrentWater > 0.0f && !bIsSpraying) {
		bIsSpraying = true;
		K2_OnSprayingChanged(bIsSpraying);

		// 서버에서만 물 소모 타이머 실행
		if (HasAuthority()) {
			GetWorldTimerManager().SetTimer(WaterTimerHandle, this, &AVRTool_Washer::ConsumeWater, WaterTimerInterval, true);
		}

	}
}

void AVRTool_Washer::StopUseTool()
{
	if (bIsSpraying) {
		bIsSpraying = false;
		K2_OnSprayingChanged(bIsSpraying);

		if (HasAuthority()) {
			GetWorldTimerManager().ClearTimer(WaterTimerHandle);
		}
	}
}



// 타이머에 의해 인터벌간격으로 실행되는 물 소모 로직
void AVRTool_Washer::ConsumeWater()
{
	if (!HasAuthority()) return;

	if (CurrentWater > 0.0f) {
		// 초당 소모량에 주기를 곱하여 차감
		CurrentWater = FMath::Max(0.0f, CurrentWater - (WaterConsumptionRate * WaterTimerInterval));


		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(400, 0.5f, FColor::Yellow,
				FString::Printf(TEXT("[Washer] Water Level: %.1f / %.1f"), CurrentWater, MaxWater));
		}



		// 라인트레이스
		FVector StartLoc = ToolMesh->GetComponentLocation();
		FVector ForwardVector = ToolMesh->GetForwardVector();

		float MaxWashDistance = 200.f;	// 임시 1.5m
		FVector EndLoc = StartLoc + (ForwardVector * MaxWashDistance);

		FHitResult HitResult;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetOwner()) {
			Params.AddIgnoredActor(GetOwner());
		}

		TArray<FHitResult> HitResults;
		bool bHit = GetWorld()->LineTraceMultiByChannel(HitResults, StartLoc, EndLoc, ECC_Visibility, Params);

		// 물줄기 디버그
		DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Blue, false, WaterTimerInterval, 0, 0.5f);

		if (bHit) {

			// 디버그로그
			if (HitResults.Num() > 0) {
				FHitResult FirstHit = HitResults[0];	// 가장먼저 맞은 물체
				if (FirstHit.GetActor()) {
					// 타격점 디버그
					DrawDebugSphere(GetWorld(), FirstHit.ImpactPoint, 10.f, 12, FColor::Red, false, WaterTimerInterval);

					// 타격한 액터 네임 디버그
					GEngine->AddOnScreenDebugMessage(100, 1.0f, FColor::Orange,
						FString::Printf(TEXT("Target : %s"), *FirstHit.GetActor()->GetName()));
				}
			}
			bool bHitDirt = false;
			bool bHitDirtyWater = false;
			AVR_Dirt* TargetDirt = nullptr;
			AVR_WaterResidue* ExistingCleanWater = nullptr;
			FHitResult TargetHitResult;	// 최종 타격 및 데칼 스폰 위치 기준점

			// 관통한 모든 물체 검사
			for (const FHitResult& Hit : HitResults) {
				AActor* HitActor = Hit.GetActor();
				if (!HitActor) continue;

				// 1. Dirt액터 타격시
				if (AVR_Dirt* HitDirt = Cast<AVR_Dirt>(HitActor)) {
					bHitDirt = true;
					TargetDirt = HitDirt;
					TargetHitResult = Hit;

					// 수압에 비례한 데미지 적용
					float CleaningDamege = 2.f * GetWaterPressureMultiplier();
					HitDirt->TakeCleaningDamage(CleaningDamege);

				}
				// 2. 기존물기 타격시
				else if (AVR_WaterResidue* WaterHit = Cast<AVR_WaterResidue>(HitActor)) {
					if (!WaterHit->bIsDirty) {
						ExistingCleanWater = WaterHit; // 깨끗한 물 기억
					}
					else {
						bHitDirtyWater = true;	// 구정물이 있음을 확인
					}
				}
				// 3. 창문 타격시
				else if (HitActor->ActorHasTag(FName("Window"))) {
					if (!bHitDirt) {
						TargetHitResult = Hit;
					}
				}
				//[추가] 플레이어 타격 시 물 위젯 생성
				else if (AVRPlayerCharacter* HitCharacter = Cast<AVRPlayerCharacter>(HitActor)) {
					if (HasAuthority()) {
						HitCharacter->Client_StartWaterHitImage();
					}
				}
			}

			// 물기 생성 로직
			if (bHitDirt && TargetDirt && WaterResidueClass)
			{
				// Dirt 하나당 DirtyWater 하나만 생성
				if (!TargetDirt->HasSpawnedDirtyWater())
				{
					FRotator DecalRot = (-TargetHitResult.ImpactNormal).Rotation();
					FVector SpawnLoc = TargetDirt->GetActorLocation();
					FTransform SpawnTransform(DecalRot, SpawnLoc);

					AVR_WaterResidue* DirtyWater =
						GetWorld()->SpawnActorDeferred<AVR_WaterResidue>(WaterResidueClass, SpawnTransform);

					if (DirtyWater)
					{
						DirtyWater->SetupWater(true);
						DirtyWater->FinishSpawning(SpawnTransform);

						TargetDirt->MarkDirtyWaterSpawned();
					}

					LastWaterSpawnLocation = TargetHitResult.ImpactPoint;
				}
			}
			else if (bHitDirtyWater) {
				// 오염물 제거되고 구정물만 남으자리 -> 아무것도 안함
			}
			else if (ExistingCleanWater) {
				// 오염물은 없고 기존 깨끗한 물이 있음 -> 수명만 3초로 리셋 (최적화)
				ExistingCleanWater->SetLifeSpan(3.0f);
			}
			else if (TargetHitResult.GetActor() && TargetHitResult.GetActor()->ActorHasTag(FName("Window")) && WaterResidueClass) {
				//  빈 창문 -> 깨끗한 물 스폰
				if (FVector::Dist(TargetHitResult.ImpactPoint, LastWaterSpawnLocation) > WaterSpawnDistanceThreshold) {
					//FActorSpawnParameters SpawnParams;
					//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					FRotator DecalRot = (-TargetHitResult.ImpactNormal).Rotation();
					FTransform SpaawnTransform(DecalRot, TargetHitResult.ImpactPoint);

					AVR_WaterResidue* CleanWater = GetWorld()->SpawnActorDeferred<AVR_WaterResidue>(WaterResidueClass, SpaawnTransform);
					if (CleanWater) {
						CleanWater->SetupWater(false); // false = 깨끗한 물 세팅
						CleanWater->FinishSpawning(SpaawnTransform);
					}
					LastWaterSpawnLocation = TargetHitResult.ImpactPoint;
				}
			}
		}
	}

	if (CurrentWater <= 0.0f) {
		StopUseTool();	// 물이 다 떨어지면 정지
	}
}


// 물 보충 함수
void AVRTool_Washer::RefillWater(float Amount)
{
	if (!HasAuthority()) return;
	CurrentWater = FMath::Clamp(CurrentWater + Amount, 0.0f, MaxWater);
}

float AVRTool_Washer::GetWaterPressureMultiplier() const
{
	if (MaxWater <= 0.0f) return 0.0f;
	// 수압 비율
	return (CurrentWater / MaxWater);
}


void AVRTool_Washer::OnRep_IsSpraying()
{
	K2_OnSprayingChanged(bIsSpraying);	// 블루프린트에서 이펙트, 사운드 등등
}

void AVRTool_Washer::ServerSetRefilling_Implementation(bool bRefilling)
{
	bIsRefilling = bRefilling;
	// 호스트(서버) 플레이어용 강제 호출
	if (GetNetMode() != NM_DedicatedServer) {
		OnRep_IsRefilling();
	}
}

void AVRTool_Washer::OnRep_IsRefilling()
{
	K2_OnRefillingChanged(bIsRefilling);
}


