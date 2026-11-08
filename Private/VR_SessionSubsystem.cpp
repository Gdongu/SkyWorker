// Fill out your copyright notice in the Description page of Project Settings.


#include "VR_SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

//Session Subsystem 초기화 (각 Session 상태에 대한 Delegate 바인딩)
UVR_SessionSubsystem::UVR_SessionSubsystem() {
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UVR_SessionSubsystem::OnCreateSessionComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UVR_SessionSubsystem::OnJoinSessionComplete);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UVR_SessionSubsystem::OnFindSessionComplete);
	UpdateSessionCompleteDelegate = FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UVR_SessionSubsystem::OnUpdateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UVR_SessionSubsystem::OnDestroySessionComplete);
}

void UVR_SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	if (GEngine) {
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	}
}

void UVR_SessionSubsystem::Deinitialize() {
	if (GEngine && NetworkFailureDelegateHandle.IsValid()) {
		GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	}

	Super::Deinitialize();
}

void UVR_SessionSubsystem::CleanupLocalSession()
{
	if (bIsCleaningUpSession) return;

	bIsCleaningUpSession = true;
	bIsFindingHost = false;

	CachedSearchResults.Empty();

	if (SessionSearch.IsValid()) {
		SessionSearch.Reset();
	}

	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();

	if (!TheSessionInterface.IsValid()) {
		bIsCleaningUpSession = false;
		ReturnToTitleMap();
		return;
	}

	TheSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	TheSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	TheSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	FNamedOnlineSession* ExistingSession = TheSessionInterface->GetNamedSession(SessionName);

	if (!ExistingSession) {
		bIsCleaningUpSession = false;
		ReturnToTitleMap();
		return;
	}

	DestroySessionCompleteDelegateHandle = TheSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	bool bDestroyStarted = TheSessionInterface->DestroySession(SessionName);

	if (!bDestroyStarted) {
		TheSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		bIsCleaningUpSession = false;
		ReturnToTitleMap();
	}
}

//세션 인터페이스 획득 (세션 검색/생성/참여/파괴를 담당)
IOnlineSessionPtr UVR_SessionSubsystem::GetSessionInterface() {
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem) {
		return nullptr;
	}

	return OnlineSubsystem->GetSessionInterface();
}

void UVR_SessionSubsystem::ReturnToTitleMap()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;

	if (!World) return;

	UGameplayStatics::OpenLevel(World,/*로비 맵 경로*/ TEXT("/Game/VRTemplate/Maps/VRTemplateMap"), true);
}

//세션 인터페이스가 있는지 확인하고 현재 세션이 있으면 세션의 이름을 획득
bool UVR_SessionSubsystem::IsInSession()
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) {
		return false;
	}
	return TheSessionInterface->GetNamedSession(SessionName) != nullptr;
}

//현재 세션이 Hosting인지 확인
bool UVR_SessionSubsystem::IsHostLobby() {
	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;

	UWorld* World = GI->GetWorld();
	if (!World) return false;

	return World->GetNetMode() == NM_ListenServer;
}

void UVR_SessionSubsystem::FindHostLobbies()
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();

	if (!TheSessionInterface.IsValid()) {
		OnFindHostSessionsComplete.Broadcast(false, TArray<FVR_HostSessionInfo>());
		return;
	}

	//이전 세션이 아직 남아있는 경우 이전 세션 제거
	if (TheSessionInterface->GetNamedSession(SessionName)) {
		CleanupLocalSession();
		return;
	}

	bIsFindingHost = true;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 50;
	SessionSearch->PingBucketSize = 50;

	FindSessionsCompleteDelegateHandle = TheSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	bool bStarted = TheSessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

	if (!bStarted) {
		bIsFindingHost = false;
		TheSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindHostSessionsComplete.Broadcast(false, TArray<FVR_HostSessionInfo>());
	}
}

void UVR_SessionSubsystem::JoinHostLobbyByIndex(int32 SearchIndex)
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}
	if (!CachedSearchResults.IsValidIndex(SearchIndex)) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	FOnlineSessionSearchResult& SelectedResult = CachedSearchResults[SearchIndex];
	FString TheHostName;
	SelectedResult.Session.SessionSettings.Get(FName(TEXT("SessionName")), TheHostName);

	if (TheHostName.IsEmpty()) {
		TheHostName = SelectedResult.Session.OwningUserName;
	}
	if (TheHostName.IsEmpty()) {
		TheHostName = FString::Printf(TEXT("Host_%d"), SearchIndex + 1);
	}

	//Client가 Host의 로비에 진입할 때 Host의 이름을 저장
	HostName = TheHostName;

	JoinSessionCompleteDelegateHandle = TheSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	bool bJoinStarted = TheSessionInterface->JoinSession(0, SessionName, SelectedResult);

	if (!bJoinStarted) {
		TheSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnFindAndJoinComplete.Broadcast(false);
	}
}

void UVR_SessionSubsystem::LeaveHostLobby()
{
	IOnlineSessionPtr OnlineSessionInterface = GetSessionInterface();

	if (OnlineSessionInterface.IsValid()) {
		OnlineSessionInterface->DestroySession(SessionName);
	}

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	CleanupLocalSession();
	UGameplayStatics::OpenLevel(World,/*로비 맵 경로*/ TEXT("/Game/VRTemplate/Maps/VRTemplateMap"), true);
}

void UVR_SessionSubsystem::CancelHostLobby()
{
	if (!IsHostLobby()) {
		return;
	}

	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();

	if (!TheSessionInterface.IsValid()) {
		ReturnToTitleMap();
		return;
	}

	if (!TheSessionInterface->GetNamedSession(SessionName)) {
		ReturnToTitleMap();
		return;
	}

	DestroySessionCompleteDelegateHandle = TheSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	bool bDestroyStarted = TheSessionInterface->DestroySession(SessionName);

	if (!bDestroyStarted) {
		TheSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		ReturnToTitleMap();
	}
}

void UVR_SessionSubsystem::CancelFindHost()
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	bIsFindingHost = false;

	if (TheSessionInterface.IsValid()) {
		TheSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		TheSessionInterface->CancelFindSessions();
	}

	if (SessionSearch.IsValid()) {
		SessionSearch.Reset();
	}

	CachedSearchResults.Empty();

	OnFindHostSessionsComplete.Broadcast(false, TArray<FVR_HostSessionInfo>());
}

//Host가 되어 Listen Server를 열기
void UVR_SessionSubsystem::OpenHostLobby(int32 Max_players, FString TheHostName)
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) {
		OnOpenHostLobbyComplete.Broadcast(false);
		return;
	}

	if (TheSessionInterface->GetNamedSession(SessionName)) {
		TheSessionInterface->DestroySession(SessionName);
	}

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = true;
	SessionSettings->NumPublicConnections = Max_players; //최대 플레이어 수
	SessionSettings->bAllowJoinInProgress = true; //현재 세션의 참여 허용
	SessionSettings->bShouldAdvertise = true;//현재 세션의 검색 허용
	SessionSettings->bUsesPresence = false; //Presence 기능 사용 여부 (친구/온라인 상태/친구 로비 참가 등)
	SessionSettings->bUseLobbiesIfAvailable = false; //로비 맵을 따로 사용할지에 대한 여부
	HostName = TheHostName;
	//Session 정보를 Hosting 상태로 변경
	SessionSettings->Set(FName(TEXT("SessionStatus")), FString(TEXT("Hosting")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(FName(TEXT("SessionName")), TheHostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteDelegateHandle = TheSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	bool bStarted = TheSessionInterface->CreateSession(0, SessionName, *SessionSettings);
	if (!bStarted) {
		//기존에 있던 Delegate 제거
		TheSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//Host 시작 실패를 Broadcast
		OnOpenHostLobbyComplete.Broadcast(false);
	}
}

void UVR_SessionSubsystem::FindAndJoinLobby()
{
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 50;
	SessionSearch->PingBucketSize = 50;

	FindSessionsCompleteDelegateHandle = TheSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	bool bStarted = TheSessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

	if (!bStarted) {
		//기존에 있던 Delegate 제거
		TheSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		//참여 실패를 BroadCast
		OnFindAndJoinComplete.Broadcast(false);
	}
}

void UVR_SessionSubsystem::OnCreateSessionComplete(FName TheSessionName, bool bSuccessful) {
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (TheSessionInterface.IsValid()) {
		TheSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	OnOpenHostLobbyComplete.Broadcast(bSuccessful);

	if (!bSuccessful) return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) return;

	UGameplayStatics::OpenLevel(World,/*로비 맵 이름*/FName("/Game/VRTemplate/Maps/VRTemplateMap"), true, TEXT("Listen"));
}

void UVR_SessionSubsystem::OnFindSessionComplete(bool bSuccessful)
{
	bIsFindingHost = false;
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();

	if (!TheSessionInterface.IsValid()) {
		OnFindHostSessionsComplete.Broadcast(false, TArray<FVR_HostSessionInfo>());
		return;
	}

	TheSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	if (!bSuccessful || !SessionSearch.IsValid()) {
		OnFindHostSessionsComplete.Broadcast(false, TArray<FVR_HostSessionInfo>());
		return;
	}

	CachedSearchResults.Empty();
	TArray<FVR_HostSessionInfo> FoundHosts;
	TSet<FString> AddedSessionIds;

	for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults) {
		if (!Result.IsValid()) continue;

		FString SessionId = Result.GetSessionIdStr();
		if (AddedSessionIds.Contains(SessionId)) continue;
		AddedSessionIds.Add(SessionId);

		if (Result.Session.NumOpenPublicConnections <= 0) continue;

		FString SessionStatus;
		Result.Session.SessionSettings.Get(FName(TEXT("SessionStatus")), SessionStatus);
		if (SessionStatus != FString(TEXT("Hosting"))) continue;

		int32 SearchIndex = CachedSearchResults.Num();
		CachedSearchResults.Add(Result);

		FVR_HostSessionInfo HostInfo;
		FString TheSessionName;
		Result.Session.SessionSettings.Get(FName(TEXT("SessionName")), TheSessionName);
		HostInfo.SearchIndex = SearchIndex;
		HostInfo.OpenSlots = Result.Session.NumOpenPublicConnections;
		HostInfo.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		HostInfo.PingInMs = Result.PingInMs;
		HostInfo.HostName = TheSessionName;
		if (HostInfo.HostName.IsEmpty()) {
			HostInfo.HostName = FString::Printf(TEXT("Host %d"), SearchIndex + 1);
		}

		FoundHosts.Add(HostInfo);
	}

	OnFindHostSessionsComplete.Broadcast(FoundHosts.Num() > 0, FoundHosts);
}

void UVR_SessionSubsystem::OnJoinSessionComplete(FName TheSessionName, EOnJoinSessionCompleteResult::Type Result) {
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	TheSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	if (Result != EOnJoinSessionCompleteResult::Success) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	FString ConnectString;
	bool bGotAddress = TheSessionInterface->GetResolvedConnectString(TheSessionName, ConnectString);

	if (!bGotAddress || ConnectString.IsEmpty()) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController) {
		OnFindAndJoinComplete.Broadcast(false);
		return;
	}

	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	bIsFindingHost = false;
	OnFindAndJoinComplete.Broadcast(true);
}

//게임이 시작되었음을 알리고 세션의 설정을 게임 시작 상태로 변경
void UVR_SessionSubsystem::MarkSessionStarted() {
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (!TheSessionInterface.IsValid()) return;

	FNamedOnlineSession* ExistingSession = TheSessionInterface->GetNamedSession(SessionName);
	if (!ExistingSession) return;

	SessionSettings = MakeShareable(new FOnlineSessionSettings(ExistingSession->SessionSettings));

	SessionSettings->bAllowJoinInProgress = false; //다른 플레이어의 참여 혀용 x
	SessionSettings->bShouldAdvertise = false; //다른 플레이어의 검색 허용 x

	SessionSettings->Set(FName("SessionStatus"), FString(TEXT("GameStarted")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UpdateSessionCompleteDelegateHandle = TheSessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegate);

	TheSessionInterface->UpdateSession(SessionName, *SessionSettings, true);
	TheSessionInterface->StartSession(SessionName);
}

void UVR_SessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) {
	CleanupLocalSession();
}

void UVR_SessionSubsystem::OnUpdateSessionComplete(FName TheSessionName, bool bSuccessful) {
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (TheSessionInterface.IsValid()) {
		TheSessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	}
}

void UVR_SessionSubsystem::OnDestroySessionComplete(FName TheSessionName, bool bSuccessful) {
	IOnlineSessionPtr TheSessionInterface = GetSessionInterface();
	if (TheSessionInterface.IsValid()) {
		TheSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	bIsCleaningUpSession = false;
	bIsFindingHost = false;

	CachedSearchResults.Empty();

	if (SessionSearch.IsValid()) {
		SessionSearch.Reset();
	}

	ReturnToTitleMap();
}