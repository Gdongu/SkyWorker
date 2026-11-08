// Fill out your copyright notice in the Description page of Project Settings.
// 액터 상속받은 클래스

#include "VR_Bucket.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "VR_BucketRefillWidget.h"
#include "TimerManager.h"

// Sets default values
AVR_Bucket::AVR_Bucket()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    /*RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;*/

    BucketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BucketMesh"));
    //BucketMesh->SetupAttachment(RootScene);
    RootComponent = BucketMesh;

    RefillWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("RefillWidgetComponent"));
    RefillWidgetComponent->SetupAttachment(BucketMesh);

    RefillWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    RefillWidgetComponent->SetDrawSize(FVector2D(400.f, 160.f));
    RefillWidgetComponent->SetTwoSided(true);
    RefillWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

    // 버킷 위쪽에 표시
    RefillWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
    RefillWidgetComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
    RefillWidgetComponent->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RefillWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RefillWidgetComponent->SetVisibility(false);

}

// Called when the game starts or when spawned
void AVR_Bucket::BeginPlay()
{
    Super::BeginPlay();

    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, -50.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

    //if (bHit && HitResult.GetComponent()) {
    //    if (BucketMesh) {
    //        BucketMesh->SetSimulatePhysics(false);
    //    }
    //    AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
    //}

    if (RefillWidgetComponent && RefillWidgetClass)
    {
        RefillWidgetComponent->SetWidgetClass(RefillWidgetClass);
        RefillWidgetComponent->InitWidget();
        RefillWidgetComponent->SetVisibility(false);
    }

    // 헤더에 FTimerHandle AttachTimerHandle; 선언하기

    // 0.1초마다 실행되는 반복 타이머를 즉석에서 람다로 정의
    GetWorldTimerManager().SetTimer(AttachTimerHandle, [this]()
        {
            // 람다 실행 시점에 내가 진짜 살아있는지 먼저 검사 (안전장치)
            if (!IsValid(this)) return;

            FVector Start = GetActorLocation();
            FVector End = Start + FVector(0.f, 0.f, -100.f);

            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

            // 레이저에 무언가 맞았고, 그게 컴포넌트라면
            if (bHit && HitResult.GetComponent())
            {
                if (BucketMesh)
                {
                    BucketMesh->SetSimulatePhysics(false);
                }

                AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);

                GetWorldTimerManager().ClearTimer(AttachTimerHandle);

                UE_LOG(LogTemp, Log, TEXT("Bucket Attached to Gondola !!!!!!!!!!!!!!!!!!!!!"));
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Finding Gondol~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
            }
        }, 0.1f, true); // 0.1초 간격으로 무한 반복(true), 곤돌라를 찾으면 해제됨

}

// Called every frame
void AVR_Bucket::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AVR_Bucket::ShowRefillWidget(float CurrentWater, float MaxWater)
{
    if (!RefillWidgetComponent || !RefillWidgetClass)
    {
        return;
    }

    if (!RefillWidgetComponent->GetWidget())
    {
        RefillWidgetComponent->SetWidgetClass(RefillWidgetClass);
        RefillWidgetComponent->InitWidget();
    }

    RefillWidgetComponent->SetVisibility(true);

    UVR_BucketRefillWidget* RefillWidget =
        Cast<UVR_BucketRefillWidget>(RefillWidgetComponent->GetUserWidgetObject());

    if (RefillWidget)
    {
        RefillWidget->UpdateRefillProgress(CurrentWater, MaxWater);
    }

    GetWorldTimerManager().ClearTimer(HideRefillWidgetTimerHandle);
    GetWorldTimerManager().SetTimer(
        HideRefillWidgetTimerHandle,
        this,
        &AVR_Bucket::HideRefillWidget,
        0.25f,
        false
    );
}

void AVR_Bucket::HideRefillWidget()
{
    if (RefillWidgetComponent)
    {
        RefillWidgetComponent->SetVisibility(false);
    }
}