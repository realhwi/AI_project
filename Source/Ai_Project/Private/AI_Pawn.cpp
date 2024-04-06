// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Pawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h" 
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
}

// Called when the game starts or when spawned
void AAI_Pawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>( Controller ))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( PlayerController->GetLocalPlayer() ))
		{
			Subsystem->AddMappingContext( IMC_AI , 0 );
		}
	}
	if (LeftHandMesh && RightHandMesh)
	{
		FVector LeftHandLocation = LeftHandMesh->GetComponentLocation();
		FVector RightHandLocation = RightHandMesh->GetComponentLocation();
        
		UE_LOG(LogTemp, Log, TEXT("Left Hand Mesh Location: %s"), *LeftHandLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("Right Hand Mesh Location: %s"), *RightHandLocation.ToString());
	}
}

// Called every frame
void AAI_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FName HandBoneName = "Hand_R";
	
    FString ReceivedData = TEXT("{\"hands\":[{\"landmarks\":[{\"id\":0,\"x\":0.5,\"y\":0.5},{\"id\":1,\"x\":0.6,\"y\":0.6}]}]}");
	ParseAndApplyHandTrackingData(ReceivedData);

	/*// 이 액터의 현재 위치를 가져옵니다.
	FVector CurrentLocation = GetActorLocation();

	// 로그에 위치 정보를 출력합니다.
	UE_LOG(LogTemp, Log, TEXT("Actor's Current Location: %s"), *CurrentLocation.ToString());*/
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
				const TArray<TSharedPtr<FJsonValue>>& Landmarks = HandValue->AsObject()->GetArrayField(TEXT("landmarks"));
				for (const TSharedPtr<FJsonValue>& Landmark : Landmarks)
				{
					TSharedPtr<FJsonObject> LandmarkObj = Landmark->AsObject();
					int32 Id = LandmarkObj->GetIntegerField(TEXT("id"));
					float X = LandmarkObj->GetNumberField(TEXT("x"));
					float Y = LandmarkObj->GetNumberField(TEXT("y"));
					float Z = 0.0f; // 예제에서는 Z 좌표를 0으로 설정

					// Mediapipe 좌표를 언리얼 좌표계로 변환
					FVector UnrealPosition = ConvertPythonToUnreal(X, Y, Z);
					
					UE_LOG(LogTemp, Log, TEXT("Landmark ID: %d, Pixel Coordinates: (X=%f, Y=%f), Unreal Coordinates: %s"), Id, X, Y, *UnrealPosition.ToString());

					FName BoneName = GetBoneNameFromLandmarkId(Id);
					if (!BoneName.IsNone())
					{
						// 여기서 BoneName.ToString()을 로그로 출력할 수 있습니다.
						UE_LOG(LogTemp, Log, TEXT("Updating Bone: %s"), *BoneName.ToString());
    
						// Id를 사용하여 UpdateHandMeshPosition을 호출
						UpdateHandMeshPosition(Id, UnrealPosition);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Bone name not found for Landmark ID: %d"), Id);
					}

					// Bone 위치 업데이트
					UpdateHandMeshPosition(Id, UnrealPosition);
				}
			}
		}
	}
}

const float PixelScale = 0.01f; // 가정: 한 픽셀이 1cm를 나타냄
FVector AAI_Pawn::ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ)
{
	// 웹캠 해상도의 중앙에서 언리얼 월드 스케일로의 변환
	// 웹캠 해상도의 중앙을 (0,0)으로 가정하고 변환합니다.
	float CenterX = 300.0f; // 600x600 해상도의 중앙 X 좌표
	float CenterY = 300.0f; // 600x600 해상도의 중앙 Y 좌표

	// 픽셀 좌표에서 중앙을 빼고 스케일 비율을 곱해서 언리얼의 위치로 변환합니다.
	float UnrealX = (PixelX - CenterX) * PixelScale;
	float UnrealY = (PixelY - CenterY) * PixelScale;
	// Z는 예시로 0을 사용합니다. 실제 애플리케이션에서는 적절한 값을 사용해야 합니다.
	float UnrealZ = 0.0f;

	return FVector(UnrealX, UnrealY, UnrealZ);
}

void AAI_Pawn::UpdateHandMeshPosition(int32 Id, const FVector& NewPosition)
{
	USkeletalMeshComponent* HandMesh = (Id == 0) ? LeftHandMesh : RightHandMesh;
	if (HandMesh)
	{
		HandMesh->SetWorldLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AAI_Pawn::UpdateBonePosition(FName BoneName, const FVector& NewPosition)
{
	// UAnimInstance를 사용하여 애니메이션 블루프린트에 IK 타겟 위치를 설정
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			// 여기서는 예시로 AnimInstance에 직접적인 함수 호출은 없으며,
			// 실제 구현은 애니메이션 블루프린트와의 인터페이스를 통해 이루어져야 합니다.
			// 예: AnimInstance->SetIKTargetPosition(TargetPosition); 가정
		}
	}
}
