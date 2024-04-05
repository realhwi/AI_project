// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "SocketClient.generated.h"

UCLASS()
class AI_PROJECT_API ASocketClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASocketClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ConnectToServer();

	bool SendData(const FString& Message);

	bool ReceiveData(FString& OutMessage);

	FSocket* Socket;
	FString Address = TEXT("127.0.0.1");
	int32 Port = 65431;
	FIPv4Address IP;
	
	bool bIsConnected;

};
