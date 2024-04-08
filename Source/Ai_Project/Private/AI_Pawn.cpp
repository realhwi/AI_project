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
		FVector CameraLocation = CameraComponent->GetComponentLocation();
		FRotator CameraRotation = CameraComponent->GetComponentRotation();
		// 카메라의 정면 방향 벡터를 가져옵니다.
		FVector ForwardVector = CameraRotation.Vector();

		// 카메라 위치에서 정면 방향으로 30cm만큼 떨어진 위치를 계산합니다.
		FVector StartPosition = CameraLocation + (ForwardVector * 30.0f);
        
		UE_LOG(LogTemp, Log, TEXT("Left Hand Mesh Location: %s"), *LeftHandLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("Right Hand Mesh Location: %s"), *RightHandLocation.ToString());
	}

	for (TActorIterator<ASocketClient> It(GetWorld(), ASocketClient::StaticClass()); It; ++It)
	{
		SocketClient = *It;
	}
}

// Called every frame
void AAI_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bHasReference)
	{
		// 기준점을 사용하여 각 랜드마크의 위치를 업데이트합니다.
		// 여기서는 예시로 핸드 메시의 손목 본 위치를 가져옵니다.
		ReferencePosition = RightHandMesh->GetSocketLocation(TEXT("wrist_inner_r"));
	}
	if(SocketClient == nullptr)
	{
		UE_LOG(LogInput,Log,TEXT("SocketClient is null") )
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
	
FName AAI_Pawn::GetBoneNameFromLandmarkId(int32 LandmarkId) const
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

void AAI_Pawn::ParseAndApplyHandTrackingData(const FString& ReceivedData)
{
	// JSON 데이터를 변환하여 핸드 랜드마크 ID와 좌표 추출해서 언리얼 엔진 월드 좌표로 변환
	UE_LOG(LogTemp, Log, TEXT("ParseAndApplyHandTrackingData called with data: %s"), *ReceivedData);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedData);
	FJsonSerializer::Deserialize(Reader, JsonObject);
	if (JsonObject.IsValid())
	{
		UE_LOG(LogInput, Log, TEXT("RawData: %s"), *ReceivedData);
		const TArray<TSharedPtr<FJsonValue>>* HandsArray;
		if (JsonObject->TryGetArrayField(TEXT("hands"), HandsArray))
		{
			for (const TSharedPtr<FJsonValue>& HandValue : *HandsArray)
			{
				const TArray<TSharedPtr<FJsonValue>>& Landmarks = HandValue->AsObject()->GetArrayField(TEXT("landmarks"));
				for (const TSharedPtr<FJsonValue>& Landmark : Landmarks)
				{
					TSharedPtr<FJsonObject> LandmarkObj = Landmark->AsObject();
					int32 Id = LandmarkObj->GetIntegerField(TEXT("id"));
					float X = LandmarkObj->GetNumberField(TEXT("x"));
					float Y = LandmarkObj->GetNumberField(TEXT("y"));
					float Z = LandmarkObj->GetNumberField(TEXT("z")); // Assuming Z coordinate is now provided and used

					// Mediapipe 좌표를 언리얼 좌표계로 변환
					FVector UnrealPosition = ConvertPythonToUnreal(X, Y, Z);
					FName BoneName = GetBoneNameFromLandmarkId(Id);
					UE_LOG(LogTemp, Log, TEXT("Landmark ID: %d, Pixel Coordinates: (X=%f, Y=%f, Z=%f), Unreal Coordinates: %s"), Id, X, Y, Z, *UnrealPosition.ToString());

					// 기준점을 가정하는 로직이 필요하다면 여기에 추가합니다.
					// 예를 들어, 손목이 기준점일 때, 기준점 로직을 적용합니다.

					// 각 랜드마크에 대한 위치 업데이트를 수행합니다.
					UpdateHandMeshPosition(Id, UnrealPosition);
				}
			}
		}
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ)
{
	// 스케일을 조정할 상수입니다. 클래스 멤버 변수가 아니라면 이름을 구체적으로 명시하는 것이 좋습니다.
	const float ConversionScale = 0.05f; 
	const float CenterX = 300.0f; 
	const float CenterY = 250.0f;
	const float ConversionScaleZ = -0.5;
	
	float UnrealX = (PixelX - CenterX) * ConversionScale;
	float UnrealY = (PixelY - CenterY) * ConversionScale;
	float UnrealZ = FMath::Pow( PixelZ  ,ConversionScaleZ); // z값 스케일을 찾기 위한 로그 찍기 

	FVector ConvertedPosition = FVector(UnrealX, UnrealY, UnrealZ);
	UE_LOG(LogActorComponent, Log, TEXT("ConvertPythonToUnreal called with PixelX: %f, PixelY: %f, PixelZ: %f"), PixelX, PixelY, PixelZ);
	UE_LOG(LogActorComponent, Log, TEXT("Converted Unreal Position: %s"), *ConvertedPosition.ToString());
	UE_LOG(LogActorComponent, Log, TEXT("ConversionScale used: %f"), ConversionScale);

	return ConvertedPosition;
}

void AAI_Pawn::UpdateHandMeshPosition(int32 Id, const FVector& NewPosition)
{
	FName BoneName = GetBoneNameFromLandmarkId(Id);
	UE_LOG(LogTemp, Log, TEXT("UpdateHandMeshPosition called with Id: %d, NewPosition: %s"), Id, *NewPosition.ToString());
	
	if (!BoneName.IsNone())
	{
		USkeletalMeshComponent* HandMesh = (Id == 0) ? LeftHandMesh : RightHandMesh;
		if (HandMesh)
		{
			int32 BoneIndex = HandMesh->GetBoneIndex(BoneName);
			if (BoneIndex != INDEX_NONE) // 본이 발견된 경우에만 진행
			{
				// 현재 본의 Transform을 얻습니다.
				FTransform BoneTransform = HandMesh->GetBoneTransform(BoneIndex);

				// 현재 본의 회전(Quaternion)을 얻습니다.
				FQuat CurrentRotation = BoneTransform.GetRotation();

				// 목표 방향을 계산합니다.
				FVector TargetDirection = (NewPosition - BoneTransform.GetLocation()).GetSafeNormal();
				FRotator TargetRotation = TargetDirection.Rotation();
                
				// 목표 회전(Quaternion)을 계산합니다.
				FQuat TargetQuatRotation = FQuat(TargetRotation);

				// Slerp를 사용하여 현재 회전에서 목표 회전으로 보간합니다.
				float Alpha = 0.1f; // 보간 계수 (0.0에서 1.0 사이의 값)
				FQuat NewQuatRotation = FQuat::Slerp(CurrentRotation, TargetQuatRotation, Alpha);

				// 위치는 그대로 두고, 보간된 회전으로 설정합니다.
				BoneTransform.SetRotation(NewQuatRotation);

				// 본의 Transform을 업데이트합니다.
				HandMesh->SetWorldTransform(BoneTransform, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
}

/*FVector AAI_Pawn::GetHandPosition(int32 HandId, FName BoneName)
{
	//TODO:
	return FVector();
}*/

FVector AAI_Pawn::GetPositionForLandmarkId(int32 LandmarkId) const
{
	const FVector* FoundPosition = LandmarkIdToPositionMap.Find(LandmarkId);
	return FoundPosition ? *FoundPosition : FVector::ZeroVector;
}
