// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Pawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "MotionControllerComponent.h"
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
	
}

// Called every frame
void AAI_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FString ReceivedData = TEXT("{...}"); // 네트워크에서 받은 JSON 문자열
	ParseAndApplyHandTrackingData(ReceivedData);
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
	default: return FName();
	}
}

void AAI_Pawn::ParseAndApplyHandTrackingData(const FString& ReceivedData)
{
	// JSON 파싱
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedData);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// JSON 데이터로부터 "hands" 배열을 추출
		const TArray<TSharedPtr<FJsonValue>>* HandsArrayPtr = nullptr;
		if (JsonObject->TryGetArrayField(TEXT("hands"), HandsArrayPtr) && HandsArrayPtr)
		{
			// 각 손에 대해 반복
			const TArray<TSharedPtr<FJsonValue>>& HandsArray = *HandsArrayPtr;
			for (const TSharedPtr<FJsonValue>& HandValue : HandsArray)
			{
				const TSharedPtr<FJsonObject>& HandObject = HandValue->AsObject();
				const TArray<TSharedPtr<FJsonValue>>& LandmarksArray = HandObject->GetArrayField(TEXT("landmarks"));

				for (const TSharedPtr<FJsonValue>& LandmarkValue : LandmarksArray)
				{
					const TSharedPtr<FJsonObject>& LandmarkObject = LandmarkValue->AsObject();
					int32 Id = LandmarkObject->GetIntegerField(TEXT("id"));
					float PythonX = LandmarkObject->GetNumberField(TEXT("x"));
					float PythonY = LandmarkObject->GetNumberField(TEXT("y"));
					// float PythonZ = LandmarkObject->GetNumberField(TEXT("z")); // 필요한 경우 사용

					// 파이썬 좌표를 언리얼 좌표계로 변환
					FVector UnrealPosition = ConvertPythonToUnreal(PythonX, PythonY);
					
					UE_LOG(LogTemp, Warning, TEXT("UnrealPosition:: %f,%f,%f,%f,%f"),PythonX, PythonY,UnrealPosition.X,UnrealPosition.Y,UnrealPosition.Z);
				}
			}
		}
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PythonX, float PythonY)
{
	float PythonMinX = 0;
	float PythonMaxX = 600;
	float UnrealMinY = -30;
	float UnrealMaxY = 30;
	float PythonMinY = 0;
	float PythonMaxY = 600;
	float UnrealMinZ = -30;
	float UnrealMaxZ = 30;

	float UnrealY = ConvertPythonValueToUnreal(PythonX, PythonMinX, PythonMaxX, UnrealMinY, UnrealMaxY);
	float UnrealZ = ConvertPythonValueToUnreal(PythonY, PythonMinY, PythonMaxY, UnrealMinZ, UnrealMaxZ);
	float UnrealX = 0; // 예시에서 단순화를 위해 0으로 설정

	return FVector(UnrealX, UnrealY, UnrealZ);
}


float AAI_Pawn::ConvertPythonValueToUnreal(float PythonValue, float PythonMin, float PythonMax, float UnrealMin, float UnrealMax)
{
    return (PythonValue - PythonMin) / (PythonMax - PythonMin) * (UnrealMax - UnrealMin) + UnrealMin;
}

	