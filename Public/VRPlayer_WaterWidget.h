// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VRPlayer_WaterWidget.generated.h"

class UImage;
class UMaterialInterface;
class UmaterialInstanceDynamic;

UCLASS()
class VR_PROJECT_API UVRPlayer_WaterWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void PlayWaterHitEffect();

	UFUNCTION(BlueprintCallable)
	bool UpdateWaterHitEffect(float DeltaTime);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WaterHitImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water Hit")
	TObjectPtr<UMaterialInstance> WaterHitMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> WaterHitMID;

	UPROPERTY(EditDefaultsOnly, Category = "Water Hit")
	float EffectDuration = 2.f;

	bool bPlaying = true;
	float EffectTime = 0.f;

};
