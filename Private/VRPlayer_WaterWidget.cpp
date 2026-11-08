// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPlayer_WaterWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVRPlayer_WaterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WaterHitImage)
	{
		UE_LOG(LogTemp, Error, TEXT("WaterHitImage is null"));
		return;
	}
	if (!WaterHitMaterial) {
		UE_LOG(LogTemp, Error, TEXT("WaterHitMaterial is null"));
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	WaterHitMID = WaterHitImage->GetDynamicMaterial();

	if (!WaterHitMID) {
		UE_LOG(LogTemp, Error, TEXT("WaterHitMID is null"));
		return;
	}

	WaterHitImage->SetVisibility(ESlateVisibility::Visible);
	WaterHitImage->SetRenderOpacity(1.f);
	WaterHitImage->SetColorAndOpacity(FLinearColor::White);
	WaterHitImage->SetDesiredSizeOverride(FVector2D(1000.f, 1000.f));

	UE_LOG(LogTemp, Warning, TEXT("Water widget construct visible test"));
}

void UVRPlayer_WaterWidget::PlayWaterHitEffect()
{
	if (!WaterHitMID || !WaterHitImage) return;

	EffectTime = 0.f;
	bPlaying = true;

	SetVisibility(ESlateVisibility::HitTestInvisible);
	WaterHitImage->SetVisibility(ESlateVisibility::HitTestInvisible);

	WaterHitImage->SetRenderOpacity(1.f);
	WaterHitImage->SetColorAndOpacity(FLinearColor::White);

	WaterHitMID->SetScalarParameterValue(TEXT("DirtOpacity"), 0.f);
}

bool UVRPlayer_WaterWidget::UpdateWaterHitEffect(float DeltaTime)
{
	if (!bPlaying || !WaterHitMID)
	{
		return true;
	}

	EffectTime += DeltaTime;

	const float SafeDuration = FMath::Max(EffectDuration, 0.01f);
	const float Progress = FMath::Clamp(EffectTime / SafeDuration, 0.f, 1.f);

	UE_LOG(LogTemp, Warning, TEXT("Water Progress: %f, Duration: %f"), Progress, EffectDuration);

	WaterHitMID->SetScalarParameterValue(TEXT("DirtOpacity"), Progress);

	if (Progress >= 1.f)
	{
		bPlaying = false;
		if (WaterHitImage)
		{
			WaterHitImage->SetVisibility(ESlateVisibility::Hidden);
		}
		SetVisibility(ESlateVisibility::Hidden);
		return true;
	}

	return false;
}
