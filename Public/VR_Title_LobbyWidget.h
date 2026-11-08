#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VR_Title_LobbyWidget.generated.h"

class UImage;
class UButton;
class UCanvasPanel;
class UVerticalBox;
class UHorizontalBox;
class UEditableTextBox;
class UVR_Title_HostSearchResultWidget;

UCLASS()
class VR_PROJECT_API UVR_Title_LobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
public:
	UFUNCTION()
	void HandleFindHostSessionsComplete(bool bSuccessful, const TArray<FVR_HostSessionInfo>& HostSessions);
	UFUNCTION()
	void HandleHostClicked();
	UFUNCTION()
	void HandleClientClicked();
	UFUNCTION()
	void HandleLobbyActionClicked();
	UFUNCTION()
	void HandleCancelClicked();
	UFUNCTION()
	void HandleHostResultClicked(int32 SearchIndex);
	UFUNCTION()
	void HandleHostNameEntered(FString Hostname);

	void UpdateMenuState();
	void RefreshPlayerIcons();
	void AddPlayerIcons();
	void RebuildHostList(const TArray<FVR_HostSessionInfo>& HostSessions);

	int32 GetCurrentLobbyPlayerCount();
	bool IsStandaloneMenu();
	bool IsHostLobby();
	bool IsClientLobby();

	void SetPlayerIconState(UImage* TargetImage, bool bActive);
	void ApplyHostName();

	bool bIsSearchingHost = false;
	bool bIsHostListOpen = false;
	bool bIsEnteringHostName = false;
private:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_HostClientButtons;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* CanvasPanel_Lobby;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Host;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Client;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LobbyAction;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HorizontalBox_PlayerIcons;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Cancel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_HostList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_HostName;

	UPROPERTY(EditAnywhere, Category = "Lobby|Player Icon", meta = (ClampMin = "1", UIMin = "1"))
	FVector2D PlayerIconSize = FVector2D(200.f, 200.f);

	UPROPERTY(EditAnywhere, Category = "Lobby|Player Icon", meta = (ClampMin = "0", UIMin = "0"))
	FMargin PlayerIconPadding = FMargin(0.f);

	UPROPERTY()
	TArray<TObjectPtr<UImage>> PlayerIcons;

	UPROPERTY(EditAnywhere, Category = "Host Search")
	TSubclassOf<UVR_Title_HostSearchResultWidget> HostSearchResultWidget;

private:
	FTimerHandle RefreshTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	TObjectPtr<UTexture2D> PlayerIconTexture;

	UPROPERTY(EditAnywhere, Category = "Lobby", meta = (ClampMin = "1", ClampMax = "2", UIMin = "1", UIMax = "2"))
	int32 Max_PlayerNum = 2;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	FLinearColor ActivePlayerColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	FLinearColor EmptyPlayerColor = FLinearColor::Gray;

public:
	FString MakeRandomHostName();
};
