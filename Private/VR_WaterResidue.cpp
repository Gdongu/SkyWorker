// Fill out your copyright notice in the Description page of Project Settings.


#include "VR_WaterResidue.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SkyGameModeBase.h"


// Sets default values
AVR_WaterResidue::AVR_WaterResidue()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetupAttachment(RootScene);
	CollisionBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionBoxComponent->SetBoxExtent(FVector(10.f, 50.f, 50.f));

	WaterMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComponent"));
	WaterMeshComponent->SetupAttachment(RootScene);


}

void AVR_WaterResidue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVR_WaterResidue, CurrentHP);
	DOREPLIFETIME(AVR_WaterResidue, NowLifeTime);
	DOREPLIFETIME(AVR_WaterResidue, bIsDirty);
}


// Called when the game starts or when spawned
void AVR_WaterResidue::BeginPlay()
{
	Super::BeginPlay();

	if (WaterMeshComponent) {
		DynamicWaterMaterial = WaterMeshComponent->CreateDynamicMaterialInstance(0);
	}

	if (DynamicWaterMaterial) {
		DynamicWaterMaterial->SetScalarParameterValue(FName("TintSwitch"), bIsDirty ? 1.f : 0.f);
		DynamicWaterMaterial->SetScalarParameterValue(FName("Angle"), FMath::RandRange(0.f, 1.f));
	}

	if (bIsDirty) {	// 오염물 위치에 생성 될 더러운 물(클리너로 닦을 것)
		MaxHP = 50.f;	//임시설정
		CurrentHP = MaxHP;
	}
	else {	// 오염물의 위치가 아닌곳에 생성 될 깨끗한 물(잠시 생겼다가 사라질 것)
		NowLifeTime = Lifetime;
		GetWorld()->GetTimerManager().ClearTimer(LifeTimeChanger);
		GetWorld()->GetTimerManager().SetTimer(LifeTimeChanger, this, &AVR_WaterResidue::LifeTimeChange, 0.1f, true, 0.1f);
	}

}

void AVR_WaterResidue::TakeWipingDamage(float DamageAmount)
{
	if (!HasAuthority() || DamageAmount < 0.f) return;

	if (CurrentHP > 0.f && bIsDirty) {
		CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);
		OnRep_CurrentHP();

		if (CurrentHP <= 10.f && !bWipeCounted)
		{
			bWipeCounted = true;

			ASkyGameModeBase* GM = GetWorld()->GetAuthGameMode<ASkyGameModeBase>();
			if (GM)
			{
				GM->WipeWaterCount++;
			}

			Destroy();
		}
	}
	else if (!bIsDirty) {
		NowLifeTime = FMath::Clamp(NowLifeTime - 0.25f, 0.f, Lifetime);
		if (NowLifeTime <= 0.1f) {
			GetWorld()->GetTimerManager().ClearTimer(LifeTimeChanger);
			Destroy();
		}
	}
}



void AVR_WaterResidue::OnRep_CurrentHP() {
	UpdateDirtVisuality();
}


void AVR_WaterResidue::SetupWater(bool bInIsDirty)
{
	if (!HasAuthority()) return;

	bIsDirty = bInIsDirty;

}


void AVR_WaterResidue::OnRep_NowLifetime()
{
	UpdateDirtVisuality();
}

void AVR_WaterResidue::LifeTimeChange()
{
	if (bIsDirty) return;
	if (NowLifeTime >= 0.1f) {
		NowLifeTime -= 0.1f;
		UpdateDirtVisuality();
	}
	else {
		GetWorld()->GetTimerManager().ClearTimer(LifeTimeChanger);
		Destroy();
	}
}

void AVR_WaterResidue::UpdateDirtVisuality() {
	if (!DynamicWaterMaterial) return;

	if (bIsDirty) {
		float HPRatio = CurrentHP / MaxHP;
		DynamicWaterMaterial->SetScalarParameterValue(FName("DirtOpacity"), 1 - HPRatio);
	}
	else {
		float LifeTimeRatio = FMath::Clamp(NowLifeTime / Lifetime, 0.f, 1.f);
		DynamicWaterMaterial->SetScalarParameterValue(FName("DirtOpacity"), 1 - LifeTimeRatio);
	}
}

// Called every frame
void AVR_WaterResidue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AVR_WaterResidue::GetWipingProgress() const
{
	if (!bIsDirty)
	{
		return 0.f;
	}

	if (MaxHP <= 0.f)
	{
		return 1.f;
	}

	return 1.f - FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f);
}

bool AVR_WaterResidue::IsWiped() const
{
	return bIsDirty && CurrentHP <= 10.f;
}