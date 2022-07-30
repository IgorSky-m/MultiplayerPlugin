// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"


//
// Declaring our own custom delegates for the menu class to bind callbacks to
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);

//Cant do dynamic because dynamic means that Objecs and structures must be comparable with blueprints but FOnlineSessionSearchResult isn't comparable to Blueprints
//We can create some structs to work with dynamic
//in multicast delegate we dont have a comma beetwen name and type like as Dynamic
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& Results, bool bWasSuccessful);

//Cant do dynamic because dynamic means that Objecs and structures must be comparable with blueprints but EOnJoinSessionCompleteResult isn't comparable to Blueprints
//We can create some structs to work with dynamic
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);



/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	UMultiplayerSessionsSubsystem();
	//
	// To handle session functionality. The menu class will call these. 
	//
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();


	//
	// Our own custom delegates for the Menu class to bind callback
	//
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
protected:

	//
	// Internal callbacks for the delegates we'll add to the Online Session Interface delegate list.
	// These don't need to be called outside this class
	//

	/**
	/* @param SessionName the name of the session this callback is for
	/* @param bWasSuccessful true if the async action completed without error, false if there was an error
	/*
	/* DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionComplete, FName, bool);
	*/
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*
	*DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsComplete, bool);
	*/
	void OnFindSessionComplete(bool bWasSuccessful);


	/**
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	* DECLARE_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionComplete, FName, EOnJoinSessionCompleteResult::Type);
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	* DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDestroySessionComplete, FName, bool);
	*/
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* @param SessionName the name of the session the that has transitioned to started
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	* DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStartSessionComplete, FName, bool);
	*/
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	//for search session
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// To add to the Online Session Interface delegate list
	// We'll bind out MultiplayerSessionsSubsystem internal callbacks to these
	//
	//FDelegateHandle - для хранения инстансов и удаления соответствующих делегатов из Handle list если они нам больше не нужны

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;


	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

	
	//support func for check is lan query or not
	const bool isLanQuery() {
		return IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";  //? true : false ;
	}

	

};
