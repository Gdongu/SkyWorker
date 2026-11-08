// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRToolBase.h"	// [추가]
#include "InputActionValue.h"
#include "VR_TitlePlayerCharacter.generated.h"

UCLASS()
class VR_PROJECT_API AVR_TitlePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVR_TitlePlayerCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 기본 VR 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UCameraComponent* VRCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* LeftHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* RightHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetInteractionComponent* RightWidgetInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetInteractionComponent* LeftWidgetInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* RightHandMesh;

	// 커스텀 로직 컴포넌트 포인터변수(Safety, Inventory)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	class UVRInventoryComponent* InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	class UVRSafetyComponent* SafetyComponent;

	// 인풋 매핑 컨텍스트를 안전하게 등록해 줄 헬퍼 함수
	void SetupInputMappingContext();

	// 에디터에서 지정 할 인풋 매핑 컨텍스트 에셋 변수
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_TitlePlayer;

	// 인풋액선
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;	// 이동

	// 테스트용 인풋액션
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Turn;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_InteractLeft;	// 왼손 상호작용

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_InteractRight;	// 오른손 상호작용


	// 테스트용 인풋액션
	void Turn(const FInputActionValue& Value);

	// 외부 노출 함수 인터페이스
	// 이동
	void Move(const FInputActionValue& Value);

	//UI 상호작용
	void PressUI(EHandType Hand);
	void ReleaseUI(EHandType Hand);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

/*추가 및 변경 사항*/
// 1. 이미 인게임용으로 만들어진 VRCharacter 코드, BP를 복사해서 새로운 VR_TitleCharacterCPP, H를 만들고 BP_VR_TitleCharacter의 Parent를 VR_TitleCharacter로 변경
// 2. IMC를 인게임용 말고 Title용으로 IMC_TitlePlayer를 별도로 만듬 (이동, 회전, 상호작용만 남기고 나머지 다 지운 형태)
// 3. 상호작용 처리를 위해 IA_UIInteractionLeft, IA_UIInteractionRight(이름은 상관없음)를 생성 (bool 값으로 받도록하고 Modifier는 없음)
// 4. BP_VR_TitleCharacter에 IMC_TitlePlayer와 IA들을 연결
// 5. Title 게임모드의 기본 Pawn에 BP_VR_TitleCharacter를 연결