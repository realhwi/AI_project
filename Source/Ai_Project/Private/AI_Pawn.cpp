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
		LeftHandMesh->SetupAttachment(SpringArmComponent);
        LeftHandMesh->SetRelativeLocation(FVector(20,-20, 0));
	}
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RightHandMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_right.SKM_MannyXR_right'"));
	if (RightHandMeshAsset.Succeeded())
	{
		RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
		RightHandMesh->SetSkeletalMesh(RightHandMeshAsset.Object);
		RightHandMesh->SetupAttachment(SpringArmComponent);
        RightHandMesh->SetRelativeLocation(FVector(20,-20, 0));
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

    FString ReceivedData = TEXT("{\"hands\":[{\"landmarks\":[{\"id\":0,\"x\":0.5,\"y\":0.5},{\"id\":1,\"x\":0.6,\"y\":0.6}]}]}");
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
	UE_LOG(LogTemp, Log, TEXT("ParseAndApplyHandTrackingData called with data: %s"), *ReceivedData);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ReceivedData);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("JSON parsing successful."));

		const TArray<TSharedPtr<FJsonValue>>* HandsArrayPtr = nullptr;
		if (JsonObject->TryGetArrayField(TEXT("hands"), HandsArrayPtr))
		{
			UE_LOG(LogTemp, Log, TEXT("Found 'hands' array in JSON."));

			for (const TSharedPtr<FJsonValue>& HandValue : *HandsArrayPtr)
			{
				const TArray<TSharedPtr<FJsonValue>>& LandmarksArray = HandValue->AsObject()->GetArrayField(TEXT("landmarks"));

				for (const TSharedPtr<FJsonValue>& LandmarkValue : LandmarksArray)
				{
					const TSharedPtr<FJsonObject>& LandmarkObject = LandmarkValue->AsObject();
					int32 Id = LandmarkObject->GetIntegerField(TEXT("id"));
					float PythonX = LandmarkObject->GetNumberField(TEXT("x"));
					float PythonY = LandmarkObject->GetNumberField(TEXT("y"));

					FVector UnrealPosition = ConvertPythonToUnreal(PythonX, PythonY);
					UpdateHandMeshPosition(Id, UnrealPosition);
					UE_LOG(LogTemp, Log, TEXT("Landmark %d: UnrealPosition = (%f, %f, %f)"), Id, UnrealPosition.X, UnrealPosition.Y, UnrealPosition.Z);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No 'hands' array found in JSON."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON."));
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PythonX, float PythonY)
{
	float PythonMinX = 0.0f, PythonMaxX = 600.0f;
	float PythonMinY = 0.0f, PythonMaxY = 600.0f;
	float UnrealMinY = -30.0f, UnrealMaxY = 30.0f;
	float UnrealMinZ = -30.0f, UnrealMaxZ = 30.0f;

	float UnrealY = FMath::GetMappedRangeValueClamped(FVector2D(PythonMinX, PythonMaxX), FVector2D(UnrealMinY, UnrealMaxY), PythonX);
	float UnrealZ = FMath::GetMappedRangeValueClamped(FVector2D(PythonMinY, PythonMaxY), FVector2D(UnrealMinZ, UnrealMaxZ), PythonY);

	return FVector(0.0f, UnrealY, UnrealZ);
}


float AAI_Pawn::ConvertPythonValueToUnreal(float PythonValue, float PythonMin, float PythonMax, float UnrealMin, float UnrealMax)
{
    return (PythonValue - PythonMin) / (PythonMax - PythonMin) * (UnrealMax - UnrealMin) + UnrealMin;
}

void AAI_Pawn::UpdateHandMeshPosition(int32 Id, const FVector& NewPosition)
{
	if (Id == 0) // 왼손의 특정 랜드마크 ID, 실제 ID 값에 따라 조정
	{
		if (LeftHandMesh)
		{
			LeftHandMesh->SetWorldLocation(NewPosition);
		}
	}
	else if (Id == 1) // 오른손의 특정 랜드마크 ID, 실제 ID 값에 따라 조정
	{
		if (RightHandMesh)
		{
			RightHandMesh->SetWorldLocation(NewPosition);
		}
	}

	if (LeftHandMesh)
	{
		FVector LeftHandWorldLocation = LeftHandMesh->GetComponentLocation();
		UE_LOG(LogTemp, Log, TEXT("Left Hand Mesh World Location: %s"), *LeftHandWorldLocation.ToString());
	}

	if (RightHandMesh)
	{
		FVector RightHandWorldLocation = RightHandMesh->GetComponentLocation();
		UE_LOG(LogTemp, Log, TEXT("Right Hand Mesh World Location: %s"), *RightHandWorldLocation.ToString());
	}
}
