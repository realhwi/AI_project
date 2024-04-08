// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketClient.h"
#include "SocketSubsystem.h"
#include "Networking.h" 
#include "Sockets.h"


// Sets default values
ASocketClient::ASocketClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASocketClient::BeginPlay()
{
	Super::BeginPlay();
	ConnectToServer();
}

// Called every frame
void ASocketClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    FString ReceivedMessage;
    if(ReceiveData(ReceivedMessage))
    {
        UE_LOG(LogTemp, Log, TEXT("Received message: %s"), *ReceivedMessage);
    }
}

void ASocketClient::ConnectToServer()
{
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (SocketSubsystem)
    {
        if (Socket == nullptr) // 소켓이 아직 생성되지 않았다면 생성합니다.
        {
            Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("default"), false);
            if (Socket == nullptr) {
                //UE_LOG(LogTemp, Error, TEXT("Failed to create socket."));
                return;
            }
        }

        // 소켓이 성공적으로 생성되었는지 확인합니다.
        if (Socket != nullptr)
        {
            // IP 주소와 포트를 사용하여 소켓 연결 시도
            bool bIsValidIP = FIPv4Address::Parse(Address, IP);
            if (bIsValidIP)
            {
                TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
                Addr->SetIp(IP.Value);
                Addr->SetPort(Port);
                bool bConnected = Socket->Connect(*Addr);
                // 연결 성공 여부에 따라 후속 처리
                if (bConnected)
                {
                    // 연결에 성공했습니다.
                    UE_LOG(LogTemp, Log, TEXT("Connected to server!"));
                }
                else
                {
                    // 연결에 실패했습니다.
                    UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server."));
                }
            }
            else
            {
                // IP 주소가 유효하지 않습니다.
                UE_LOG(LogTemp, Warning, TEXT("Invalid IP Address."));
            }
        }
        else
        {
            // 소켓 생성에 실패했습니다.
            UE_LOG(LogTemp, Warning, TEXT("Could not create socket."));
        }
    }
    else
    {
        // 소켓 시스템 서브시스템을 가져오지 못했습니다.
        UE_LOG(LogTemp, Error, TEXT("Could not get socket subsystem."));
    }
}

bool ASocketClient::SendData(const FString& Message)
{
    if(!Socket || Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
    {
        return false;
    }
    
    // 문자열을 UTF-8로 인코딩합니다.
    FTCHARToUTF8 Convert(*Message);
    int32 BytesSent = 0;
    
    // 데이터를 전송하고 성공 여부를 반환합니다.
    bool bSuccess = Socket->Send((uint8*)Convert.Get(), Convert.Length(), BytesSent);
    return bSuccess;
}

bool ASocketClient::ReceiveData(FString& OutMessage)
{
    if (!Socket || Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
    {
        return false;
    }

    // 버퍼 사이즈를 정의합니다.
    const int32 BufferSize = 16384;
    uint8 ReceiveBuffer[BufferSize];
    int32 BytesRead = 0;
    
    // 데이터를 수신하고 성공 여부를 확인합니다.
    bool bHasData = Socket->HasPendingData((uint32&)BytesRead);
    if (bHasData && BytesRead > 0) // 데이터가 있고, 읽을 바이트가 있으면
    {
        bool bReceived = Socket->Recv(ReceiveBuffer, BufferSize, BytesRead, ESocketReceiveFlags::None);
        if (bReceived && BytesRead > 0)
        {
            // 수신한 데이터를 FString으로 변환합니다.
            // UTF8_TO_TCHAR로 변환할 때 실제로 읽은 바이트만큼만 처리합니다.
            FString ReceivedString = FString(UTF8_TO_TCHAR(reinterpret_cast<char*>(ReceiveBuffer))).Left(BytesRead);
            OutMessage = ReceivedString;
            UE_LOG(LogTemp, Log, TEXT("%s"), *OutMessage); // 로그 레벨을 Error에서 Log로 변경
            return true;
        }
    }

    return false; // 데이터 수신에 실패하면 false 반환
}






