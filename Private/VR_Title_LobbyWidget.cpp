// Fill out your copyright notice in the Description page of Project Settings.


#include "VR_Title_LobbyWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "VR_Title_HostSearchResultWidget.h"
#include "TimerManager.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"

//여기는 기존 파일의 이름에 따라 변경
#include "VR_SessionSubsystem.h"
#include "VR_Title_GameMode.h"

void UVR_Title_LobbyWidget::NativeConstruct() {
	Super::NativeConstruct();

	//버튼 이벤트 바인딩
	if (Button_Host) {
		Button_Host->OnPressed.AddDynamic(this, &ThisClass::HandleHostClicked);
		Button_Host->OnClicked.AddDynamic(this, &ThisClass::HandleHostClicked);
	}
	if (Button_Client) {
		Button_Client->OnPressed.AddDynamic(this, &ThisClass::HandleClientClicked);
		Button_Client->OnClicked.AddDynamic(this, &ThisClass::HandleClientClicked);
	}
	if (Button_LobbyAction) {
		Button_LobbyAction->OnPressed.AddDynamic(this, &ThisClass::HandleLobbyActionClicked);
		Button_LobbyAction->OnClicked.AddDynamic(this, &ThisClass::HandleLobbyActionClicked);
	}
	if (Button_Cancel) {
		Button_Cancel->OnPressed.AddDynamic(this, &ThisClass::HandleCancelClicked);
		Button_Cancel->OnClicked.AddDynamic(this, &ThisClass::HandleCancelClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance) {
		if (UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>()) {
			SessionSubsystem->OnFindHostSessionsComplete.AddDynamic(this, &ThisClass::HandleFindHostSessionsComplete);
		}
	}

	//플레이어 아이콘을 로비 위젯에 추가
	AddPlayerIcons();
	//Widget 초기화
	UpdateMenuState();
	//플레이어 아이콘 초기화
	RefreshPlayerIcons();

	//플레이어 아이콘 갱신 타이머 설정
	if (UWorld* World = GetWorld()) {
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &ThisClass::RefreshPlayerIcons, 0.3f, true);
	}

	// 로비에 표시되는 플레이어 아이콘 최대 2개로 고정
	Max_PlayerNum = FMath::Clamp(Max_PlayerNum, 1, 2);
}

void UVR_Title_LobbyWidget::NativeDestruct() {
	if (UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	//버튼 이벤트 바인딩 해제
	if (Button_Host) {
		Button_Host->OnPressed.RemoveDynamic(this, &ThisClass::HandleHostClicked);
		Button_Host->OnClicked.RemoveDynamic(this, &ThisClass::HandleHostClicked);
	}

	if (Button_Client) {
		Button_Client->OnPressed.RemoveDynamic(this, &ThisClass::HandleClientClicked);
		Button_Client->OnClicked.RemoveDynamic(this, &ThisClass::HandleClientClicked);
	}

	if (Button_LobbyAction) {
		Button_LobbyAction->OnPressed.RemoveDynamic(this, &ThisClass::HandleLobbyActionClicked);
		Button_LobbyAction->OnClicked.RemoveDynamic(this, &ThisClass::HandleLobbyActionClicked);
	}

	if (Button_Cancel) {
		Button_Cancel->OnPressed.RemoveDynamic(this, &ThisClass::HandleCancelClicked);
		Button_Cancel->OnClicked.RemoveDynamic(this, &ThisClass::HandleCancelClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance) {
		if (UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>()) {
			SessionSubsystem->OnFindHostSessionsComplete.RemoveDynamic(this, &ThisClass::HandleFindHostSessionsComplete);
		}
	}

	Super::NativeDestruct();
}

void UVR_Title_LobbyWidget::HandleFindHostSessionsComplete(bool bSuccessful, const TArray<FVR_HostSessionInfo>& HostSessions)
{
	bIsSearchingHost = false;

	if (!bSuccessful || HostSessions.Num() <= 0) {
		bIsHostListOpen = false;

		if (Button_Host) {
			Button_Host->SetIsEnabled(true);
		}
		if (Button_Client) {
			Button_Client->SetIsEnabled(false);
		}
		if (VerticalBox_HostList) {
			VerticalBox_HostList->ClearChildren();
		}

		UpdateMenuState();
		return;
	}

	bIsHostListOpen = true;

	RebuildHostList(HostSessions);

	if (VerticalBox_HostList) {
		VerticalBox_HostList->SetVisibility(ESlateVisibility::Visible);
	}

	UpdateMenuState();
}

//호스트로 시작 버튼을 눌렀을 때
void UVR_Title_LobbyWidget::HandleHostClicked()
{
	//호스트,클라이언트 시작 버튼 비활성화
	if (Button_Host) {
		Button_Host->SetIsEnabled(false);
	}
	if (Button_Client) {
		Button_Client->SetIsEnabled(false);
	}

	// 닉네임을 입력받지 않고 랜덤하게 생성하도록 변경
	bIsEnteringHostName = false;
	FString RandomHostName = MakeRandomHostName();
	HandleHostNameEntered(RandomHostName);
}

void UVR_Title_LobbyWidget::HandleClientClicked() {
	//호스트,클라이언트 시작 버튼 비활성화
	if (Button_Host) {
		Button_Host->SetIsEnabled(false);
	}
	if (Button_Client) {
		Button_Client->SetIsEnabled(false);
	}

	bIsSearchingHost = true;
	bIsHostListOpen = false;
	UpdateMenuState();

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>();
	if (!SessionSubsystem) return;

	//다른 빈 자리가 있는 호스트 검색
	SessionSubsystem->FindHostLobbies();
}

//로비의 버튼을 클릭했을 때
void UVR_Title_LobbyWidget::HandleLobbyActionClicked()
{
	//호스트라면 게임 시작 시도
	if (IsHostLobby()) {
		AVR_Title_GameMode* TitleGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AVR_Title_GameMode>() : nullptr;
		if (TitleGameMode) {
			TitleGameMode->StartGameFromLobby();
		}

		return;
	}
	//클라이언트라면 로비에서 나가기
	if (IsClientLobby()) {
		UGameInstance* GameInstance = GetGameInstance();
		if (!GameInstance) return;

		UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>();
		if (!SessionSubsystem) return;

		SessionSubsystem->LeaveHostLobby();
	}
}

void UVR_Title_LobbyWidget::HandleCancelClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UVR_SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UVR_SessionSubsystem>();

	if (!SessionSubsystem) return;

	if (IsHostLobby()) {
		SessionSubsystem->CancelHostLobby();
		return;
	}

	if (IsClientLobby()) {
		SessionSubsystem->LeaveHostLobby();
		return;
	}

	if (bIsEnteringHostName) {
		bIsEnteringHostName = false;
		UpdateMenuState();
		return;
	}

	if (bIsSearchingHost || bIsHostListOpen) {
		SessionSubsystem->CancelFindHost();

		bIsSearchingHost = false;
		bIsHostListOpen = false;
		bIsEnteringHostName = false;

		if (CanvasPanel_Lobby) {
			CanvasPanel_Lobby->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (VerticalBox_HostList) {
			VerticalBox_HostList->ClearChildren();
			VerticalBox_HostList->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (Button_Host) {
			Button_Host->SetIsEnabled(true);
		}
		if (Button_Client) {
			Button_Client->SetIsEnabled(true);
		}

		UpdateMenuState();
		return;
	}
}

void UVR_Title_LobbyWidget::HandleHostResultClicked(int32 SearchIndex)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>();

	if (!SessionSubsystem) return;

	SessionSubsystem->JoinHostLobbyByIndex(SearchIndex);
}

void UVR_Title_LobbyWidget::HandleHostNameEntered(FString Hostname)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UVR_SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UVR_SessionSubsystem>();

	if (!SessionSubsystem) return;

	bIsEnteringHostName = false;
	//호스트 로비 열기
	SessionSubsystem->OpenHostLobby(Max_PlayerNum, Hostname);
}

//위젯 상태를 갱신
void UVR_Title_LobbyWidget::UpdateMenuState() {
	UWorld* World = GetWorld();
	if (!World) return;

	bool bStandalone = IsStandaloneMenu();
	bool bHost = IsHostLobby();
	bool bClient = IsClientLobby();
	//호스트/클라이언트 선택을 하지 않은 경우
	if (bStandalone && !bIsSearchingHost && !bIsHostListOpen && !bIsEnteringHostName) {
		if (CanvasPanel_Lobby) {
			CanvasPanel_Lobby->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (VerticalBox_HostClientButtons) {
			VerticalBox_HostClientButtons->SetVisibility(ESlateVisibility::Visible);
		}
		if (VerticalBox_HostList) {
			VerticalBox_HostList->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (EditableTextBox_HostName) {
			EditableTextBox_HostName->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_LobbyAction) {
			Button_LobbyAction->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (HorizontalBox_PlayerIcons) {
			HorizontalBox_PlayerIcons->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_Cancel) {
			Button_Cancel->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_Host) {
			Button_Host->SetIsEnabled(true);
		}
		if (Button_Client) {
			Button_Client->SetIsEnabled(true);
		}
		return;
	}
	//Client가 Host를 검색/선택 중인 경우
	else if (bStandalone && (bIsSearchingHost || bIsHostListOpen) && !bIsEnteringHostName) {
		if (CanvasPanel_Lobby) {
			CanvasPanel_Lobby->SetVisibility(ESlateVisibility::Visible);
		}
		if (VerticalBox_HostClientButtons) {
			VerticalBox_HostClientButtons->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (VerticalBox_HostList) {
			VerticalBox_HostList->SetVisibility(ESlateVisibility::Visible);
		}
		if (EditableTextBox_HostName) {
			EditableTextBox_HostName->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_LobbyAction) {
			Button_LobbyAction->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (HorizontalBox_PlayerIcons) {
			HorizontalBox_PlayerIcons->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_Cancel) {
			Button_Cancel->SetVisibility(ESlateVisibility::Visible);
		}
		if (Button_Host) {
			Button_Host->SetIsEnabled(false);
		}
		if (Button_Client) {
			Button_Client->SetIsEnabled(false);
		}
		return;
	}

	//Host또는 Client가 로비에 있는 경우
	else {
		if (CanvasPanel_Lobby) {
			CanvasPanel_Lobby->SetVisibility(ESlateVisibility::Visible);
		}
		if (VerticalBox_HostClientButtons) {
			VerticalBox_HostClientButtons->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (Button_LobbyAction) {
			Button_LobbyAction->SetVisibility(bHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		if (EditableTextBox_HostName) {
			EditableTextBox_HostName->SetVisibility(ESlateVisibility::Visible);
			ApplyHostName();
		}
		if (VerticalBox_HostList) {
			VerticalBox_HostList->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (HorizontalBox_PlayerIcons) {
			HorizontalBox_PlayerIcons->SetVisibility(ESlateVisibility::Visible);
		}
		if (Button_Cancel) {
			Button_Cancel->SetVisibility(ESlateVisibility::Visible);
		}
		if (Button_Host) {
			Button_Host->SetIsEnabled(false);
		}
		if (Button_Client) {
			Button_Client->SetIsEnabled(false);
		}
	}

}

void UVR_Title_LobbyWidget::RefreshPlayerIcons()
{
	//호스트/클라이언트 로비 상태가 아니라면 갱신 무시
	if (IsStandaloneMenu()) return;

	UpdateMenuState();
	int32 CurrentPlayerCount = GetCurrentLobbyPlayerCount();

	//현재 로비의 플레이어의 수만큼 아이콘 활성화
	for (int32 i = 0; i < PlayerIcons.Num(); i++) {
		bool bActive = i < CurrentPlayerCount;
		SetPlayerIconState(PlayerIcons[i], bActive);
	}

}

void UVR_Title_LobbyWidget::AddPlayerIcons()
{
	PlayerIcons.Empty();
	if (!HorizontalBox_PlayerIcons || !WidgetTree) {
		return;
	}

	HorizontalBox_PlayerIcons->ClearChildren();

	// Horizontal Box에 플레이어 아이콘을 최대 2개까지만 생성
	const int32 IconCount = FMath::Clamp(Max_PlayerNum, 1, 2);

	for (int32 Index = 0; Index < IconCount; ++Index) {
		USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(*FString::Printf(TEXT("PlayerIconSizeBox_%d"), Index + 1))
		);

		UImage* NewPlayerIcon = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			FName(*FString::Printf(TEXT("PlayerIcon_%d"), Index + 1))
		);

		if (!IconSizeBox || !NewPlayerIcon) {
			continue;
		}

		IconSizeBox->SetWidthOverride(PlayerIconSize.X);
		IconSizeBox->SetHeightOverride(PlayerIconSize.Y);

		if (PlayerIconTexture) {
			NewPlayerIcon->SetBrushFromTexture(PlayerIconTexture, true);
			NewPlayerIcon->SetBrushSize(PlayerIconSize);
		}

		NewPlayerIcon->SetDesiredSizeOverride(PlayerIconSize);
		NewPlayerIcon->SetColorAndOpacity(EmptyPlayerColor);

		IconSizeBox->AddChild(NewPlayerIcon);

		UHorizontalBoxSlot* IconSlot = HorizontalBox_PlayerIcons->AddChildToHorizontalBox(IconSizeBox);
		if (IconSlot) {
			IconSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			IconSlot->SetHorizontalAlignment(HAlign_Center);
			IconSlot->SetVerticalAlignment(VAlign_Center);
			IconSlot->SetPadding(PlayerIconPadding);
		}

		PlayerIcons.Add(NewPlayerIcon);
	}
}

void UVR_Title_LobbyWidget::RebuildHostList(const TArray<FVR_HostSessionInfo>& HostSessions)
{
	if (!VerticalBox_HostList || !HostSearchResultWidget) return;

	VerticalBox_HostList->ClearChildren();

	for (const FVR_HostSessionInfo& HostInfo : HostSessions) {
		UVR_Title_HostSearchResultWidget* ResultWidget = CreateWidget<UVR_Title_HostSearchResultWidget>(GetOwningPlayer(), HostSearchResultWidget);

		if (!ResultWidget) continue;

		ResultWidget->SetupHostEntry(HostInfo);
		ResultWidget->OnSearchedHostClicked.AddDynamic(this, &ThisClass::HandleHostResultClicked);

		VerticalBox_HostList->AddChild(ResultWidget);
	}
}

//현재 로비의 플레이어 수를 획득
int32 UVR_Title_LobbyWidget::GetCurrentLobbyPlayerCount()
{
	UWorld* World = GetWorld();
	if (!World) return 0;

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState) return 0;

	return GameState->PlayerArray.Num();
}

//Session이 연결되지 않은 상태인지 확인
bool UVR_Title_LobbyWidget::IsStandaloneMenu()
{
	UWorld* World = GetWorld();
	if (!World) return true;

	return World->GetNetMode() == NM_Standalone;
}

//자신의 Session이 호스트인지 확인
bool UVR_Title_LobbyWidget::IsHostLobby() {
	UWorld* World = GetWorld();
	if (!World) return false;

	return World->GetNetMode() == NM_ListenServer;
}

//자신의 Session이 클라이언트인지 확인
bool UVR_Title_LobbyWidget::IsClientLobby()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	return World->GetNetMode() == NM_Client;
}

//플레이어 아이콘 상태(색상) 변경
void UVR_Title_LobbyWidget::SetPlayerIconState(UImage* TargetImage, bool bActive)
{
	if (!TargetImage) return;

	TargetImage->SetColorAndOpacity(bActive ? ActivePlayerColor : EmptyPlayerColor);
}

void UVR_Title_LobbyWidget::ApplyHostName()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	UVR_SessionSubsystem* SessionSubsystem =
		GameInstance->GetSubsystem<UVR_SessionSubsystem>();
	if (!SessionSubsystem) return;

	FString TheHostname = SessionSubsystem->HostName;
	if (TheHostname.IsEmpty()) {
		TheHostname = TEXT("HostRoom");
	}

	if (EditableTextBox_HostName) {
		EditableTextBox_HostName->SetText(FText::FromString(TheHostname));
		EditableTextBox_HostName->SetIsReadOnly(true);
	}
}

//[추가] 방 이름 랜덤하게 생성
FString UVR_Title_LobbyWidget::MakeRandomHostName()
{
	int32 RandomNumber = FMath::RandRange(0, 9999);
	return FString::Printf(TEXT("Host_%04d"), RandomNumber);
}

