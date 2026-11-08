// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VRGameInstance.generated.h"

UCLASS()
class VR_PROJECT_API UVRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	bool bIsWasherUnlocked = false;
	bool bIsCleanerUnlocked = false;
	float SavedWasherWater = 100.f;
	bool bIsWatchEquipped = false;

	bool bHasHelmet = false;
	bool bHasHarness = false;
	bool bHasHook = false;
	bool bSafetyPenaltyApplied = false;
};