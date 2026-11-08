// Fill out your copyright notice in the Description page of Project Settings.


#include "VR_Dirt.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AVR_Dirt::AVR_Dirt()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 충돌체 설정
	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBoxComponent->SetupAttachment(RootScene);
	CollisionBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionBoxComponent->SetBoxExtent(FVector(10.f, 50.f, 50.f));

	//먼지 매쉬 컴포넌트 설정
	DirtMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dirt"));
	DirtMeshComponent->SetupAttachment(RootScene);
	DirtMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentHP = MaxHP;
}

void AVR_Dirt::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVR_Dirt, CurrentHP);
}

void AVR_Dirt::TakeCleaningDamage(float DamageAmount)
{
	// 서버에서만 연산 수행
	if (!HasAuthority()) return;

	if (CurrentHP > 0.f) {
		// 체력 차감
		CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

		UpdateDirtVisuality();
		ForceNetUpdate();

		// 오퍼시티가 0에 가까울 때 너무 안보여서 임시로 20체력기준으로 설정(기존0)
		if (CurrentHP <= 5.f) {
			Destroy();
		}
	}
}

// Called when the game starts or when spawned
void AVR_Dirt::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시, 데칼에 입혀진 머티리얼을 코드로 조작할 수 있도록 다이내믹 인스턴스로 변환
	if (DirtMeshComponent) {
		DynamicMeshMaterial = DirtMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	}

	UpdateDirtVisuality();
}

void AVR_Dirt::OnRep_CurrentHP()
{
	UpdateDirtVisuality();
}


// Called every frame
void AVR_Dirt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVR_Dirt::UpdateDirtVisuality()
{
	if (DynamicMeshMaterial) {
		if (CurrentHP < 0.f || MaxHP <= 0.f) return;
		// 체력 비율
		float HPRatio = CurrentHP / MaxHP;

		// 투명도 조절 : 파라미터 이름"Opacity"로 설정
		DynamicMeshMaterial->SetScalarParameterValue(FName("DirtOpacity"), 1 - HPRatio);

		//  물 젖음 연출: 체력이 깎일수록 Roughness(거칠기)를 0(매끈함)에 가깝게 만들어서 물광이 나게 함
		// 시작 시 완전 거칠음(1.0) -> 체력이 다 닳아갈 때쯤 물에 젖어 반짝거림(0.1)
		float RoughnessValue = FMath::Lerp(0.1f, 1.0f, HPRatio);
		DynamicMeshMaterial->SetScalarParameterValue(FName("Roughness"), RoughnessValue);
	}
}

float AVR_Dirt::GetCleaningProgress() const
{
	if (MaxHP <= 0.f) return 1.f;

	return 1.f - FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f);
}

bool AVR_Dirt::IsCleaned() const
{
	return CurrentHP <= 5.f;
}