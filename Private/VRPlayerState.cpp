// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPlayerState.h"
#include "Net/UnrealNetwork.h"

AVRPlayerState::AVRPlayerState()
{
	Money = 700000.f;
}

void AVRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVRPlayerState, Money);
}

void AVRPlayerState::Wage()
{
	if (!HasAuthority()) return;
	if (Money >= 70000.f)
	{
		Money -= 70000.f;
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Wind -70000 | Player: %d | Remaining: %.0f"), GetPlayerId(), Money);
	}
}

FString AVRPlayerState::GetGrade() const
{
	if (Money >= 700000.f)
		return TEXT("A");
	else if (Money >= 650000.f)
		return TEXT("B");
	else if (Money >= 600000.f)
		return TEXT("C");
	else
		return TEXT("F");
}

void AVRPlayerState::ApplyPenalty(float Amount)
{
	if (!HasAuthority()) return;
	Money = FMath::Max(0.f, Money - Amount);
}