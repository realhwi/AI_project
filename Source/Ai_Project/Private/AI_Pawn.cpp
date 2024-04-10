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
#include "Kismet/BlueprintTypeConversions.h"


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
	// 
	if (LeftHandMesh && RightHandMesh)
	{
		FVector LeftHandLocation = LeftHandMesh->GetComponentLocation();
		FVector RightHandLocation = RightHandMesh->GetComponentLocation();
        
		UE_LOG(LogTemp, Log, TEXT("Left Hand Mesh Location: %s"), *LeftHandLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("Right Hand Mesh Location: %s"), *RightHandLocation.ToString());
	}

	for (TActorIterator<ASocketClient> It(GetWorld(), ASocketClient::StaticClass()); It; ++It)
	{
		SocketClient = *It;
	}
	
	if (CameraComponent && LeftHandMesh)
	{
		// 카메라 기준 손의 초기 상대 위치 저장
		InitialHandLocation = CameraComponent->GetComponentTransform().InverseTransformPosition(LeftHandMesh->GetComponentLocation());
		InitialHandRotation = LeftHandMesh->GetComponentRotation() - CameraComponent->GetComponentRotation();
		bInitialHandPositionSet = true;
	}
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
			for (const TSharedPtr<FJsonValue>& HandValue : *HandsArray)
			{
				FString HandType = HandValue->AsObject()->GetStringField(TEXT("type")); // 손 타입 (Left or Right)

				const TArray<TSharedPtr<FJsonValue>>& Landmarks = HandValue->AsObject()->GetArrayField(TEXT("landmarks"));
				for (const TSharedPtr<FJsonValue>& Landmark : Landmarks)
				{
					TSharedPtr<FJsonObject> LandmarkObj = Landmark->AsObject();
					int32 Id = LandmarkObj->GetIntegerField(TEXT("id"));
					float X = LandmarkObj->GetNumberField(TEXT("x"));
					float Y = LandmarkObj->GetNumberField(TEXT("y"));
					float Z = LandmarkObj->GetNumberField(TEXT("z"));

					FVector UnrealPosition = ConvertPythonToUnreal(X, Y, Z);

					// 손의 타입에 따라 메시 업데이트
					if (HandType.Equals("Left", ESearchCase::IgnoreCase))
					{
						UpdateHandMeshPosition(Id, UnrealPosition, "Left");
					}
					else if (HandType.Equals("Right", ESearchCase::IgnoreCase))
					{
						UpdateHandMeshPosition(Id, UnrealPosition, "Right");
					}
				}
			}
		}
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ)
{
	// 스케일을 조정할 상수입니다. 클래스 멤버 변수가 아니라면 이름을 구체적으로 명시하는 것이 좋습니다.
	const float ConversionScale = 0.05f; 
	const float CenterX =320.0f; 
	const float CenterY = 240.0f;
	const float ConversionScaleZ = 0;
	
	float UnrealX = 0;
	float UnrealY = (PixelX - CenterX) * ConversionScale;
	float UnrealZ = (PixelY - CenterY) * ConversionScale;

	FVector ConvertedPosition = FVector(UnrealX, UnrealY, UnrealZ);
	UE_LOG(LogActorComponent, Log, TEXT("ConvertPythonToUnreal called with PixelX: %f, PixelY: %f, PixelZ: %f"), PixelX, PixelY, PixelZ);
	UE_LOG(LogActorComponent, Log, TEXT("Converted Unreal Position: %s"), *ConvertedPosition.ToString());
	UE_LOG(LogActorComponent, Log, TEXT("ConversionScale used: %f"), ConversionScale);

	return ConvertedPosition;
}

void AAI_Pawn::UpdateHandMeshPosition(int32 Id, const FVector& NewPosition, const FString& HandType)
{
	if (!CameraComponent || !LeftHandMesh || !RightHandMesh) return;

	// HandType에 따른 메시 선택
	USkeletalMeshComponent* HandMesh = HandType.Equals("Left", ESearchCase::IgnoreCase) ? LeftHandMesh : RightHandMesh;

	FName BoneName = GetBoneNameFromLandmarkId(Id, HandType);
	if (BoneName.IsNone() || !HandMesh) return;

	int32 BoneIndex = HandMesh->GetBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return; // 유효한 본 인덱스가 아니면 반환

	// WorldPosition은 NewPosition을 세계 좌표계로 변환한 것입니다.
	FVector WorldPosition = CameraComponent->GetComponentTransform().TransformPosition(NewPosition);

	// 본의 현재 위치와 회전을 가져옵니다.
	FTransform CurrentTransform = HandMesh->GetBoneTransform(BoneIndex);
	FVector CurrentPosition = CurrentTransform.GetLocation();
	FQuat CurrentRotation = CurrentTransform.GetRotation();

	// 목표 방향을 계산합니다.
	FVector Direction = (WorldPosition - CurrentPosition).GetSafeNormal();
	FRotator TargetRotation = Direction.Rotation();

	// FRotator에서 FQuat으로 변환
	FQuat TargetQuatRotation = FQuat(TargetRotation);

	// 보간을 사용해 새 회전을 계산합니다.
	float InterpSpeed = 5.0f; // 조정 가능한 보간 속도
	FQuat NewRotation = FQuat::Slerp(CurrentRotation, TargetQuatRotation, GetWorld()->GetDeltaSeconds() * InterpSpeed);

	// 새 위치와 회전으로 본의 Transform을 설정합니다.
	HandMesh->SetWorldLocationAndRotation(WorldPosition, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogTemp, Log, TEXT("Updated %s Hand Mesh Position to %s and Rotation"), *HandType, *WorldPosition.ToString());
}

FVector AAI_Pawn::GetPositionForLandmarkId(int32 LandmarkId) const
{
	const FVector* FoundPosition = LandmarkIdToPositionMap.Find(LandmarkId);
	return FoundPosition ? *FoundPosition : FVector::ZeroVector;
}

