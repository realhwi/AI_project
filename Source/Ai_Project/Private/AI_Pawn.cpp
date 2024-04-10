// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Pawn.h"

#include "ComponentReregisterContext.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h" 
#include "SocketClient.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AAI_Pawn::AAI_Pawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 0.f; 
	SpringArmComponent->bUsePawnControlRotation = true; 
	SpringArmComponent->bInheritPitch = false; 
	SpringArmComponent->bInheritRoll = false; 
	SpringArmComponent->bInheritYaw = true; 

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	FVector HandRelativeLocation = FVector(0, 0, -60);
	
	//Hand Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> LeftHandMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	if (LeftHandMeshAsset.Succeeded())
	{
		LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
		LeftHandMesh->SetSkeletalMesh(LeftHandMeshAsset.Object);
		LeftHandMesh->SetupAttachment(CameraComponent);
		LeftHandMesh->SetRelativeLocation(HandRelativeLocation);
	}
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RightHandMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_right.SKM_MannyXR_right'"));
	if (RightHandMeshAsset.Succeeded())
	{
		RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
		RightHandMesh->SetSkeletalMesh(RightHandMeshAsset.Object);
		RightHandMesh->SetupAttachment(CameraComponent);
		RightHandMesh->SetRelativeLocation(HandRelativeLocation);
	}
	//SocketClient = CreateDefaultSubobject<ASocketClient> (TEXT("SocketClient")); // ASocketClient 인스턴스를 얻는 코드
	
}

// Called when the game starts or when spawned
void AAI_Pawn::BeginPlay()
{
	Super::BeginPlay();
	
	ReferencePosition = FVector::ZeroVector;
	bHasReference = false;
	
	// 인풋 매핑 컨트롤 
	if (APlayerController* PlayerController = Cast<APlayerController>( Controller ))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( PlayerController->GetLocalPlayer() ))
		{
			Subsystem->AddMappingContext( IMC_AI , 0 );
		}
	}
	for (TActorIterator<ASocketClient> It(GetWorld(), ASocketClient::StaticClass()); It; ++It)
	{
		SocketClient = *It;
	}
	// 카메라의 초기 위치 저장
	InitialCameraLocation = CameraComponent->GetComponentLocation();
    
	// 오른손 핸드 메시의 초기 위치 저장 및 카메라로부터의 상대적인 거리 계산
	InitialRightHandLocation = RightHandMesh->GetComponentLocation();
	HandMeshOffsetFromCamera = InitialRightHandLocation - InitialCameraLocation;

	// 로그 출력
	UE_LOG(LogTemp, Log, TEXT("Camera Location at BeginPlay: %s"), *InitialCameraLocation.ToString());
	UE_LOG(LogTemp, Log, TEXT("Right Hand Mesh Initial Location: %s"), *InitialRightHandLocation.ToString());
	UE_LOG(LogTemp, Log, TEXT("Hand Mesh Offset From Camera: %s"), *HandMeshOffsetFromCamera.ToString());
}

// Called every frame
void AAI_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bHasReference)
	{
		ReferencePosition = RightHandMesh->GetSocketLocation(TEXT("wrist_inner_r"));
	}
	if(SocketClient != nullptr)
	{
		FString ReceivedData ;
		SocketClient->ReceiveData(ReceivedData);
		UE_LOG(LogInput,Log,TEXT("recData:%s"),*ReceivedData)
		ParseAndApplyHandTrackingData(ReceivedData);
	}
}

// Called to bind functionality to input
void AAI_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAI_Pawn::RotateCamera);
	}
}

void AAI_Pawn::RotateCamera(const FInputActionValue& Value)
{
	if (Controller)
	{
		FRotator NewRotation = Controller->GetControlRotation() + FRotator(0.f, Value.GetMagnitude(), 0.f);
		Controller->SetControlRotation(NewRotation);
	}
}
	
FName AAI_Pawn::GetBoneNameFromLandmarkId(int32 LandmarkId, const FString& HandType) const
{
	 // 오른손에 대한 본 이름 매핑
    if (HandType == "Right")
    {
        switch (LandmarkId)
        {
        case 0: return FName(TEXT("wrist_inner_r"));
        case 1: return FName(TEXT("thumb_01_r"));
        case 2: return FName(TEXT("thumb_02_r"));
        case 3: return FName(TEXT("thumb_03_r"));
        case 5: return FName(TEXT("index_01_r"));
        case 6: return FName(TEXT("index_02_r"));
        case 7: return FName(TEXT("index_03_r"));
        case 9: return FName(TEXT("middle_01_r"));
        case 10: return FName(TEXT("middle_02_r"));
        case 11: return FName(TEXT("middle_03_r"));
        case 13: return FName(TEXT("pinky_01_r"));
        case 14: return FName(TEXT("pinky_02_r"));
        case 15: return FName(TEXT("pinky_03_r"));
        case 17: return FName(TEXT("ring_01_r"));
        case 18: return FName(TEXT("ring_02_r"));
        case 19: return FName(TEXT("ring_03_r"));
        default: return FName(); // ID가 매핑되지 않은 경우, 빈 FName 반환
        }
    }
    // 왼손에 대한 본 이름 매핑
    else if (HandType == "Left")
    {
        switch (LandmarkId)
        {
        case 0: return FName(TEXT("wrist_inner_l"));
        case 1: return FName(TEXT("thumb_01_l"));
        case 2: return FName(TEXT("thumb_02_l"));
        case 3: return FName(TEXT("thumb_03_l"));
        case 5: return FName(TEXT("index_01_l"));
        case 6: return FName(TEXT("index_02_l"));
        case 7: return FName(TEXT("index_03_l"));
        case 9: return FName(TEXT("middle_01_l"));
        case 10: return FName(TEXT("middle_02_l"));
        case 11: return FName(TEXT("middle_03_l"));
        case 13: return FName(TEXT("pinky_01_l"));
        case 14: return FName(TEXT("pinky_02_l"));
        case 15: return FName(TEXT("pinky_03_l"));
        case 17: return FName(TEXT("ring_01_l"));
        case 18: return FName(TEXT("ring_02_l"));
        case 19: return FName(TEXT("ring_03_l"));
        default: return FName(); // ID가 매핑되지 않은 경우, 빈 FName 반환
        }
    }

    // 예외 처리: 만약 HandType이 "Right" 또는 "Left"가 아닐 경우
    return FName(); // 빈 FName 반환
}

void AAI_Pawn::ParseAndApplyHandTrackingData(const FString& ReceivedData)
{
	UE_LOG(LogTemp, Log, TEXT("ParseAndApplyHandTrackingData called with data: %s"), *ReceivedData);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedData);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* HandsArray;
		if (JsonObject->TryGetArrayField(TEXT("hands"), HandsArray))
		{
			for (const auto& HandValue : *HandsArray)
			{
				FString HandType = HandValue->AsObject()->GetStringField(TEXT("type"));
				auto& Landmarks = HandValue->AsObject()->GetArrayField(TEXT("landmarks"));

				for (const auto& Landmark : Landmarks)
				{
					auto LandmarkObj = Landmark->AsObject();
					int32 Id = LandmarkObj->GetIntegerField(TEXT("id"));
					float X = LandmarkObj->GetNumberField(TEXT("x"));
					float Y = LandmarkObj->GetNumberField(TEXT("y"));
					float Z = LandmarkObj->GetNumberField(TEXT("z"));

					FVector UnrealPosition = ConvertPythonToUnreal(X, Y, Z);
					UE_LOG(LogTemp, Log, TEXT("Converted Unreal Position for %s Hand ID %d: %s"), *HandType, Id, *UnrealPosition.ToString());

					// 초기 손 위치에 대한 웹캠 데이터의 상대 위치 적용
					FVector InitialHandLocation = (HandType.Equals("Left", ESearchCase::IgnoreCase)) ? InitialLeftHandLocation : InitialRightHandLocation;
					FVector NewHandPosition = InitialHandLocation + (UnrealPosition - InitialHandLocation);
					UE_LOG(LogTemp, Log, TEXT("New Hand Position for %s Hand ID %d: %s"), *HandType, Id, *NewHandPosition.ToString());

					UpdateHandMeshPosition(Id, NewHandPosition, HandType);
				}
			}
		}
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ)
{
	// 웹캠 해상도의 최대값을 정의합니다.
	const float MaxWebcamX = 600.0f;
	const float MaxWebcamY = 580.0f;

	// 웹캠 해상도의 중앙값을 정의합니다.
	const float CenterX = MaxWebcamX / 2.0f;
	const float CenterY = MaxWebcamY / 2.0f;

	// 웹캠에서의 픽셀 단위를 언리얼 엔진의 월드 스케일로 변환할 스케일 인자를 정의합니다.
	const float ConversionScaleXY = 0.05f; // x와 y축 변환에 사용될 스케일 인자
	const float ConversionScaleZ = 0.05f; // z축 변환에 사용될 스케일 인자

	// 웹캠의 픽셀 좌표를 언리얼의 월드 좌표계로 변환합니다.
	// 여기서, 웹캠의 x축은 언리얼의 y축으로, 웹캠의 y축은 언리얼의 z축으로 매핑되며,
	// 웹캠의 z축은 언리얼의 x축으로 매핑됩니다.
	float UnrealY = (PixelX - CenterX) * ConversionScaleXY; // 웹캠 왼쪽이 마이너스, 오른쪽이 플러스 방향
	float UnrealZ = (CenterY - PixelY) * ConversionScaleXY; // 웹캠 상단이 마이너스, 하단이 플러스 방향
	float UnrealX = PixelZ * ConversionScaleZ; // 웹캠에 가까울수록 언리얼에서 멀어지는 방향(양의 X 방향)

	FVector ConvertedPosition = FVector(UnrealX, UnrealY, UnrealZ);
	UE_LOG(LogActorComponent, Log, TEXT("ConvertPythonToUnreal called with PixelX: %f, PixelY: %f, PixelZ: %f"), PixelX, PixelY, PixelZ);
	UE_LOG(LogActorComponent, Log, TEXT("Converted Unreal Position: %s"), *ConvertedPosition.ToString());
	UE_LOG(LogActorComponent, Log, TEXT("ConversionScale used: XY: %f, Z: %f"), ConversionScaleXY, ConversionScaleZ);

	return ConvertedPosition;
}

void AAI_Pawn::UpdateHandMeshPosition(int32 Id, const FVector& NewPosition, const FString& HandType)
{
	if (!CameraComponent || !LeftHandMesh || !RightHandMesh) return;

	USkeletalMeshComponent* HandMesh = HandType.Equals("Left", ESearchCase::IgnoreCase) ? LeftHandMesh : RightHandMesh;

	FName BoneName = GetBoneNameFromLandmarkId(Id, HandType);
	if (BoneName.IsNone() || !HandMesh) return;
	
	// 웹캠 데이터 기반으로 계산된 핸드 메시의 새로운 위치를 계산합니다.
	// 이때, HandMeshOffsetFromCamera를 사용하여 카메라 위치에 상대적인 위치를 고려합니다.
	FVector NewHandPositionRelativeToCamera = NewPosition + HandMeshOffsetFromCamera;
	FVector NewWorldPosition = CameraComponent->GetComponentLocation() + NewHandPositionRelativeToCamera;

	// 카메라의 회전을 가져와서 손의 회전을 조정합니다.
	FRotator CameraRotator = CameraComponent->GetComponentRotation();
	FRotator HandRotation = FRotator(-CameraRotator.Pitch, CameraRotator.Yaw, -CameraRotator.Roll);
	FQuat TargetQuat = FQuat(HandRotation);

	// 현재 핸드 메시의 회전에서 목표 회전으로 보간
	FQuat CurrentQuat = HandMesh->GetComponentQuat();
	FQuat NewQuat = FQuat::Slerp(CurrentQuat, TargetQuat, RotationSpeed);
	
	// 핸드 메시의 새로운 위치와 조정된 회전으로 업데이트합니다.
	HandMesh->SetWorldLocationAndRotation(NewWorldPosition, NewQuat);

    UE_LOG(LogTemp, Log, TEXT("Updated %s Hand Mesh Position to %s and Adjusted Rotation"), *HandType, *NewWorldPosition.ToString());
}

void AAI_Pawn::UpdateBonePosition(int32 BoneId, const FVector& NewPosition, const FString& HandType)
{
	if (!CameraComponent || !LeftHandMesh || !RightHandMesh) return;

	// 해당 핸드 타입에 맞는 메시 컴포넌트를 선택합니다.
	USkeletalMeshComponent* HandMesh = HandType.Equals("Left", ESearchCase::IgnoreCase) ? LeftHandMesh : RightHandMesh;

	// 본 이름을 ID로부터 가져옵니다.
	FName BoneName = GetBoneNameFromLandmarkId(BoneId, HandType);
	if (BoneName.IsNone() || !HandMesh) return;

	// 본의 현재 월드 위치를 가져옵니다.
	FVector BoneWorldPosition = HandMesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);

	// 웹캠 데이터에 기반하여 본의 새로운 월드 위치를 계산합니다.
	FVector AdjustedNewPosition = InitialCameraLocation + (NewPosition - ReferencePosition);

	// 핸드 메시 컴포넌트에서 본의 위치를 업데이트합니다.
	HandMesh->SetBoneLocationByName(BoneName, AdjustedNewPosition, EBoneSpaces::WorldSpace);

	UE_LOG(LogTemp, Log, TEXT("Updated %s Bone Position to %s"), *BoneName.ToString(), *AdjustedNewPosition.ToString());
}
