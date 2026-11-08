// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/EngineBaseTypes.h"
#include "VR_SessionSubsystem.generated.h"

//호스트를 선택하기 위한 Host Session의 정보
USTRUCT(BlueprintType)
struct VR_PROJECT_API FVR_HostSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	int32 OpenSlots = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SearchIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFindHostSessionsComplete, bool, bSuccessful, const TArray<FVR_HostSessionInfo>&, HostSessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRSessionResult, bool, bSuccessful);

UCLASS()
class VR_PROJECT_API UVR_SessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UVR_SessionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void CleanupLocalSession();

	UFUNCTION(BlueprintCallable, Category = "VR_Session")
	void OpenHostLobby(int32 Max_players, FString HostName);

	UFUNCTION(BlueprintCallable, Category = "VR_Session")
	void FindAndJoinLobby();

	UFUNCTION(BlueprintCallable, Category = "VR_Session")
	void MarkSessionStarted();

	UFUNCTION(BlueprintPure, Category = "VR_Session")
	bool IsInSession();

	UFUNCTION(BlueprintPure, Category = "VR_Session")
	bool IsHostLobby();

	UFUNCTION(BlueprintCallable)
	void FindHostLobbies();

	UFUNCTION(BlueprintCallable)
	void JoinHostLobbyByIndex(int32 SearchIndex);

	UPROPERTY(BlueprintAssignable)
	FOnFindHostSessionsComplete OnFindHostSessionsComplete;

	UFUNCTION(BlueprintCallable, Category = "VR_Session")
	void LeaveHostLobby();

	UFUNCTION(BlueprintCallable, Category = "VR_Session")
	void CancelHostLobby();

	UFUNCTION(BlueprintCallable)
	void CancelFindHost();

	UFUNCTION(BlueprintPure)
	bool IsFindingHost() { return bIsFindingHost; }

	UPROPERTY(BlueprintAssignable, Category = "VR_Session")
	FVRSessionResult OnOpenHostLobbyComplete;

	UPROPERTY(BlueprintAssignable, Category = "VR_Session")
	FVRSessionResult OnFindAndJoinComplete;

	UPROPERTY(BlueprintReadWrite, Category = "VR_Session")
	int32 Total_Players = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VR_Session")
	FString HostName;

private:
	IOnlineSessionPtr GetSessionInterface();
	void ReturnToTitleMap();

	void OnCreateSessionComplete(FName TheSessionName, bool bSuccessful);
	void OnFindSessionComplete(bool bSuccessful);
	void OnJoinSessionComplete(FName TheSessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnUpdateSessionComplete(FName TheSessionName, bool bSuccessful);
	void OnDestroySessionComplete(FName TheSessionName, bool bSuccessful);

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	bool bIsFindingHost = false;
	bool bIsCleaningUpSession = false;
protected:
	IOnlineSessionPtr SessionInterface;
	FName SessionName = NAME_GameSession;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TArray<FOnlineSessionSearchResult> CachedSearchResults;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FDelegateHandle NetworkFailureDelegateHandle;
};

/*
*
Online Subsystem을 사용하기 위해선 Build.cs에 다음을 추가
"EnhancedInput", "HeadMountedDisplay", "OnlineSubsystem", "OnlineSubsystemUtils"
Widget사용을 위해 다음을 추가
"UMG"

또한 Private쪽에 아래 코드도 추가
PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" }); <--

또한 아래의 코드도 추가
DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
*
*/



/*VR Game Multi Merge 단계*/
/*
1. GameInstanceSubsystem을 부모로 VR_SessionSubsystem와 VR_Game_Title_GameMode의 cpp, h 파일 생성
2. 각 파일 중 맵의 경로와 이름을 재설정
3. Build.cs에 코드 추가 (상단에 있음)
4. Complie 후 VR_SessionSubsystem, VR_Game_Title_GameMode의 BP 생성
5. BP_VR_Game_Title_GameMode를 WorldSetting에 세팅 (Title 맵에 세팅)
6. BP_VRGameMode(기본내장(다른 gamemode가 있다면 그것을 사용))을 WorldSetting에 세팅 (튜토리얼 맵, 게임 맵에 세팅)
7. UserWidget을 부모로 VR_Title_LobbyWidget cpp. h 파일 생성 (이미 타이틀 화면 UI가 있다면 코드만 추가)
8. Build.cs에 코드 추가
9. VR_Title_LobbyWidget을 컴파일 하고 이후 WBP_VR_TitleLobbyWidget을 추가
10. 아래와 같이 UI 이름 설정 (이미 설정된 것이 있다면 WBP의 각 Widget 이름 변경)
-VerticalBox_HostClientButtons
	-Button_Host;
	-Button_Client
-Panel_Lobby
	-Image_BackGround(필요하면 추가)
	-Button_LobbyAction
	-Image_PlayerIcon1
	-Image_PlayerIcon2
	-Image_PlayerIcon3
11. WBP의 PlayerIcons에 각 Image_PlayerIcon들을 바인딩
12. PlayerController를 부모로 VR_Title_PlayerController cpp, h 파일 생성
13. 컴파일 후 BP_VR_Title_PlayerController 생성
14. Title 맵에 세팅된 GameMode에 VR_Title_PlayerController를 설정
15. Host 목록에 들어갈 위젯인 VR_Title_HostResultWidget h, cpp 생성
16. 컴파일 후 BP_VR_Title_HostResultWidget 생성
17. BP_VR_Title_LobbyWidget의 HostSearchResultWidget에 BP_VR_Title_HostResultWidget 등록

UI의 크기/배치는 따로 적용하지 않음, 모든 BP와 CPP/h의 이름은 변경 가능
*/