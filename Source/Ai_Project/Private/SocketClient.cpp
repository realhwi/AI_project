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
        if (Socket == nullptr) // ������ ���� �������� �ʾҴٸ� �����մϴ�.
        {
            Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("default"), false);
            if (Socket == nullptr) {
                //UE_LOG(LogTemp, Error, TEXT("Failed to create socket."));
                return;
            }
        }

        // ������ ���������� �����Ǿ����� Ȯ���մϴ�.
        if (Socket != nullptr)
        {
            // IP �ּҿ� ��Ʈ�� ����Ͽ� ���� ���� �õ�
            bool bIsValidIP = FIPv4Address::Parse(Address, IP);
            if (bIsValidIP)
            {
                TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
                Addr->SetIp(IP.Value);
                Addr->SetPort(Port);
                bool bConnected = Socket->Connect(*Addr);
                // ���� ���� ���ο� ���� �ļ� ó��
                if (bConnected)
                {
                    // ���ῡ �����߽��ϴ�.
                    UE_LOG(LogTemp, Log, TEXT("Connected to server!"));
                }
                else
                {
                    // ���ῡ �����߽��ϴ�.
                    UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server."));
                }
            }
            else
            {
                // IP �ּҰ� ��ȿ���� �ʽ��ϴ�.
                UE_LOG(LogTemp, Warning, TEXT("Invalid IP Address."));
            }
        }
        else
        {
            // ���� ������ �����߽��ϴ�.
            UE_LOG(LogTemp, Warning, TEXT("Could not create socket."));
        }
    }
    else
    {
        // ���� �ý��� ����ý����� �������� ���߽��ϴ�.
        UE_LOG(LogTemp, Error, TEXT("Could not get socket subsystem."));
    }
}

bool ASocketClient::SendData(const FString& Message)
{
    if(!Socket || Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
    {
        return false;
    }
    
    // ���ڿ��� UTF-8�� ���ڵ��մϴ�.
    FTCHARToUTF8 Convert(*Message);
    int32 BytesSent = 0;
    
    // �����͸� �����ϰ� ���� ���θ� ��ȯ�մϴ�.
    bool bSuccess = Socket->Send((uint8*)Convert.Get(), Convert.Length(), BytesSent);
    return bSuccess;
}

bool ASocketClient::ReceiveData(FString& OutMessage)
{
    if (!Socket || Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
    {
        return false;
    }

    // ���� ����� �����մϴ�.
    const int32 BufferSize = 16384;
    uint8 ReceiveBuffer[BufferSize];
    int32 BytesRead = 0;
    
    // �����͸� �����ϰ� ���� ���θ� Ȯ���մϴ�.
    bool bHasData = Socket->HasPendingData((uint32&)BytesRead);
    if (bHasData && BytesRead > 0) // �����Ͱ� �ְ�, ���� ����Ʈ�� ������
    {
        bool bReceived = Socket->Recv(ReceiveBuffer, BufferSize, BytesRead, ESocketReceiveFlags::None);
        if (bReceived && BytesRead > 0)
        {
            // ������ �����͸� FString���� ��ȯ�մϴ�.
            // UTF8_TO_TCHAR�� ��ȯ�� �� ������ ���� ����Ʈ��ŭ�� ó���մϴ�.
            FString ReceivedString = FString(UTF8_TO_TCHAR(reinterpret_cast<char*>(ReceiveBuffer))).Left(BytesRead);
            OutMessage = ReceivedString;
            UE_LOG(LogTemp, Log, TEXT("%s"), *OutMessage); // �α� ������ Error���� Log�� ����
            return true;
        }
    }

    return false; // ������ ���ſ� �����ϸ� false ��ȯ
}






