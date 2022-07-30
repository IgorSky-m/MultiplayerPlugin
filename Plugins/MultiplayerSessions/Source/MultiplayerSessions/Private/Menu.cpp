// Fill out your copyright notice in the Description page of Project Settings.

//TODO 1 29.07.2022 Рефактор и перенос в основной модуль MultiplayerSubsystem
//TODO 2 29.07.2022 если строка null?
//TODO 3 29.07.2022 Проверить все ->SetIsEnabled(true) и ->SetIsEnabled(false);
#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(MenuLog, All, All)

bool UMenu::Initialize() 
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}


	return true;
}

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeofMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeofMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();

	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if (PlayerController)
		{
			//Input mode focused on UI only
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	//bind Custom Delegates 
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem) 
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
		
	}

}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}


void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();

	if (!World) 
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();


	if (!PlayerController)
	{
		return;
	}
	//Input mode focused on Game only
	FInputModeGameOnly InputModeData;

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}


void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful) 
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session created Successfully"))
			);
		}


		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Faled to create session"))
			);
		}

		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& Results, bool bWasSuccessful)
{
	if (!bWasSuccessful || Results.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
		return;
	}

	if (MultiplayerSessionsSubsystem == nullptr)
	{
		//??
		JoinButton->SetIsEnabled(true);
		return;
	}
	
	for (auto& Result : Results)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	//??
	JoinButton->SetIsEnabled(true);

}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{

	if (EOnJoinSessionCompleteResult::Success != Result) {
		//TODO 3 29.07.2022 Проверить все ->SetIsEnabled(true) и ->SetIsEnabled(false);
		JoinButton->SetIsEnabled(true);
		return;
	}

	//TODO 1 29.07.2022 Рефактор и перенос в основной модуль MultiplayerSubsystem
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		auto SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid()) {
			FString Address;
			//Получаем строку адреса для коннекта (ип или подобное)
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			//Получаем контроллер игрока
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

			//TODO 2 29.07.2022 если строка null?
			//Если ок, коннектимся
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Session started status: %b"), bWasSuccessful)
		);
	}
}
