#include "VR_Title_PlayerController.h"
#include "VR_Title_LobbyWidget.h"

void AVR_Title_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (!VR_Title_LobbyWidget) return;

	LobbyWidget = CreateWidget<UVR_Title_LobbyWidget>(this, VR_Title_LobbyWidget);
	if (!LobbyWidget) return;

	//---------------Test용 모니터 관련 코드---------------------
	LobbyWidget->AddToViewport();
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	//------------------------------------------------------------

	InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
}
