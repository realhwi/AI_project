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

void AAI_Pawn::UpdateHandTracking(const FString& HandData)
{
	// FName BoneName = GetBoneNameFromLandmarkId(LandmarkId, bIsLeftHand);
	// if (BoneName.IsNone()) return;
	//
	// // 좌표 변환 로직 구현. 예를 들어, cm 단위로 스케일 조정 등
	// FVector UnrealPosition = ConvertPositionToUnreal(Position);
	//
	// // 본 위치 업데이트. 이 예에서는 컴포넌트 공간에서의 위치 설정을 가정합니다.
	// USkeletalMeshComponent* MeshComponent = GetMesh();
	// if (MeshComponent)
	// {
	// 	MeshComponent->SetBoneLocationByName(BoneName, UnrealPosition, EBoneSpaces::ComponentSpace);
	// }
}

// 예시: Unreal Engine 좌표계로 변환하는 함수
FVector ConvertToUnrealPosition(FVector PythonPosition)
{
	// PythonPosition은 파이썬에서 전송된 랜드마크 위치
	// 실제 좌표계 변환 로직을 구현해야 합니다.
	return FVector(PythonPosition.X, PythonPosition.Y, PythonPosition.Z);
}

// 본의 위치를 업데이트하는 로직
void UpdateBonePosition(FName BoneName, FVector NewPosition)
{
	// // 예시 매핑. 실제 ID와 본 이름을 프로젝트에 맞게 조정해야 합니다.
	// switch (LandmarkId)
	// {
	// case 0: return bIsLeftHand ? FName(TEXT("wrist_l")) : FName(TEXT("wrist_r"));
	// case 4: return bIsLeftHand ? FName(TEXT("thumb_01_l")) : FName(TEXT("thumb_01_r"));
	// 	// 다른 손가락 본에 대한 ID 매핑 추가...
	// default: return FName();
	// }
}
