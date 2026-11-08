#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "VRPlayerState.generated.h"

UCLASS()
class VR_PROJECT_API AVRPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AVRPlayerState();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Wage")
	float Money;

	void Wage();

	bool bHasHelmet = false;
	bool bHasHerness = false;
	bool bHasHook = false;
	bool bSafetyPenaltyApplied = false;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Wage")
	FString GetGrade() const;

	void ApplyPenalty(float Amount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
