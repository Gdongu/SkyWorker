// Fill out your copyright notice in the Description page of Project Settings.

#include "SkyGameState.h"
#include "Net/UnrealNetwork.h"

void ASkyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASkyGameState, ElapsedSeconds);
	DOREPLIFETIME(ASkyGameState, SprayProgress);
	DOREPLIFETIME(ASkyGameState, WipeProgress);
	DOREPLIFETIME(ASkyGameState, bWorkComplete);
	DOREPLIFETIME(ASkyGameState, GondolaFloor);
}

void ASkyGameState::OnRep_ElapsedSeconds()
{
	// 클라이언트에서 시간이 바뀔 때 처리할 것이 있으면 여기에 추가
}

FText ASkyGameState::GetFormattedTime() const
{
	int32 Minutes = ElapsedSeconds / 60;
	int32 Seconds = ElapsedSeconds % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}
