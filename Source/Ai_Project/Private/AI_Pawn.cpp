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
		EnhancedInputComponent->BindAction(IA_Look_Right, ETriggerEvent::Started, this, &AAI_Pawn::R_RotateCamera);
		EnhancedInputComponent->BindAction(IA_Look_Left, ETriggerEvent::Started, this, &AAI_Pawn::L_RotateCamera);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAI_Pawn::Move);
	}
}

void AAI_Pawn::R_RotateCamera(const FInputActionValue& Value)
{
	if (Controller)
	{
		FRotator CurrentRotation = Controller->GetControlRotation();
		FRotator NewRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw + 90.0f, CurrentRotation.Roll);
		Controller->SetControlRotation(NewRotation);
	}
}

void AAI_Pawn::L_RotateCamera(const FInputActionValue& Value)
{
	if (Controller)
	{
		FRotator CurrentRotation = Controller->GetControlRotation();
		FRotator NewRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw -90.0f, CurrentRotation.Roll);
		Controller->SetControlRotation(NewRotation);
	}
}

void AAI_Pawn::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation( 0 , Rotation.Yaw , 0 );

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::X );

		// get right vector 
		const FVector RightDirection = FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::Y );

		// add movement 
		AddMovementInput( ForwardDirection , MovementVector.Y );
		AddMovementInput( RightDirection , MovementVector.X );
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

				FVector TotalPosition = FVector::ZeroVector;
				int32 Count = 0;
				this->LocalBoneIdToPositionMap.Empty();
				
				for (const auto& Landmark : Landmarks)
				{
					auto LandmarkObj = Landmark->AsObject();
					int32 Id = LandmarkObj->GetIntegerField(TEXT("id"));
					float X = LandmarkObj->GetNumberField(TEXT("x"));
					float Y = LandmarkObj->GetNumberField(TEXT("y"));
					float Z = LandmarkObj->GetNumberField(TEXT("z"));

					FVector UnrealPosition = ConvertPythonToUnreal(X, Y, Z);
					
					// 초기 손 위치에 대한 웹캠 데이터의 상대 위치 적용
					FVector InitialHandLocation = (HandType.Equals("Left", ESearchCase::IgnoreCase)) ? InitialLeftHandLocation : InitialRightHandLocation;
					FVector NewHandPosition = InitialHandLocation + (UnrealPosition - InitialHandLocation);
					UE_LOG(LogTemp, Log, TEXT("New Hand Position for %s Hand ID %d: %s"), *HandType, Id, *NewHandPosition.ToString());

					this->LocalBoneIdToPositionMap.Add(Id, NewHandPosition);
					
					// 위치와 카운트를 업데이트
					TotalPosition += UnrealPosition;
					Count++;
				}
				if (Count > 0)
				{
					FVector AveragePosition = TotalPosition / static_cast<float>(Count);
					FRotator AverageRotation; // 평균 회전 계산 로직 필요
					UpdateHandMeshPosition(HandType, AveragePosition, AverageRotation);
					// UpdateBonePositions(this->LocalBoneIdToPositionMap, HandType);
				}
			}
		}
	}
}

FVector AAI_Pawn::ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ)
{
	// 웹캠 해상도의 최대값을 정의
	const float MaxWebcamX = 600.0f;
	const float MaxWebcamY = 580.0f;

	// 웹캠 해상도의 중앙값을 정의
	const float CenterX = MaxWebcamX / 2.0f;
	const float CenterY = MaxWebcamY / 2.0f;

	// 웹캠에서의 픽셀 단위를 언리얼 엔진의 월드 스케일로 변환할 스케일 인자를 정의
	const float ConversionScaleXY = 0.05f; // x와 y축 변환에 사용될 스케일 인자
	const float ConversionScaleZ = 100.f; // z축 변환에 사용될 스케일 인자

	// 웹캠의 픽셀 좌표를 언리얼의 월드 좌표계로 변환
	// 여기서, 웹캠의 x축은 언리얼의 y축으로, 웹캠의 y축은 언리얼의 z축으로 매핑되며,
	// 웹캠의 z축은 언리얼의 x축으로 매핑
	
	float UnrealY = (PixelX - CenterX) * ConversionScaleXY * 2 + 15; // 웹캠 왼쪽이 마이너스, 오른쪽이 플러스 방향
	float UnrealZ = (CenterY - PixelY) * ConversionScaleXY; // 웹캠 상단이 마이너스, 하단이 플러스 방향
	float UnrealX = PixelZ * ConversionScaleZ; // 웹캠에 가까울수록 언리얼에서 멀어지는 방향(양의 X 방향)

	FVector ConvertedPosition = FVector(UnrealX, UnrealY, UnrealZ);
	UE_LOG(LogActorComponent, Log, TEXT("ConvertPythonToUnreal called with PixelX: %f, PixelY: %f, PixelZ: %f"), PixelX, PixelY, PixelZ);
	UE_LOG(LogActorComponent, Log, TEXT("Converted Unreal Position: %s"), *ConvertedPosition.ToString());
	UE_LOG(LogActorComponent, Log, TEXT("ConversionScale used: XY: %f, Z: %f"), ConversionScaleXY, ConversionScaleZ);

	return ConvertedPosition;
}

void AAI_Pawn::UpdateHandMeshPosition(const FString& HandType, const FVector& NewPosition, const FRotator& NewRotation)
{
	if (!CameraComponent || !LeftHandMesh || !RightHandMesh) return;

	USkeletalMeshComponent* HandMesh = nullptr;
	if (HandType.Equals("Left", ESearchCase::IgnoreCase)) {
		HandMesh = LeftHandMesh;
	} else if (HandType.Equals("Right", ESearchCase::IgnoreCase)) {
		HandMesh = RightHandMesh;
	}
	if (!HandMesh) return;
	
	// 웹캠 데이터 기반으로 계산된 핸드 메시의 새로운 위치를 계산
	// 이때, HandMeshOffsetFromCamera를 사용하여 카메라 위치에 상대적인 위치를 고려
	FVector NewHandPositionRelativeToCamera = NewPosition + HandMeshOffsetFromCamera;
	FVector NewWorldPosition = CameraComponent->GetComponentLocation() + NewHandPositionRelativeToCamera;

	// 카메라의 회전을 가져와서 손의 회전을 조정
	FRotator CameraRotator = CameraComponent->GetComponentRotation();
	FRotator HandRotation = FRotator(-CameraRotator.Pitch, CameraRotator.Yaw, -CameraRotator.Roll);
	// 손 메시의 최종 회전을 결정하기 위해 카메라 회전과 손의 실제 회전을 보간
	FQuat TargetQuat = FQuat(HandRotation); 
	FQuat CurrentQuat = HandMesh->GetComponentQuat();
	FQuat NewQuat = FQuat::Slerp(CurrentQuat, TargetQuat, RotationSpeed);
	
	// 핸드 메시의 새로운 위치와 조정된 회전으로 업데이트합니다.
	HandMesh->SetWorldLocationAndRotation(NewWorldPosition , NewQuat);
	HandMesh->SetRelativeRotation( FRotator(HandMesh->GetRelativeRotation().Pitch,270,HandMesh->GetRelativeRotation().Roll));
	
    UE_LOG(LogTemp, Log, TEXT("Updated %s Hand Mesh Position to %s and Adjusted Rotation"), *HandType, *NewWorldPosition.ToString());
}

void AAI_Pawn::UpdateBonePositions(const TMap<int32, FVector>& BoneIdToPositionUpdateMap, const FString& HandType)
{
	USkeletalMeshComponent* HandMesh = nullptr;
	if (HandType.Equals("Left", ESearchCase::IgnoreCase)) {
		HandMesh = LeftHandMesh;
	} else if (HandType.Equals("Right", ESearchCase::IgnoreCase)) {
		HandMesh = RightHandMesh;
	}

	if (!HandMesh) return;

	for (const auto& Pair : BoneIdToPositionUpdateMap)
	{
		int32 BoneId = Pair.Key;
		FVector TargetPosition = Pair.Value;

		FName BoneName = GetBoneNameFromLandmarkId(BoneId, HandType);
		if (BoneName.IsNone()) continue;

		// 본의 현재 인덱스를 찾습니다.
		int32 BoneIndex = HandMesh->GetBoneIndex(BoneName);
		if (BoneIndex != INDEX_NONE)
		{
			// 본의 현재 변환을 가져옵니다.
			FTransform BoneTransform = HandMesh->GetBoneTransform(BoneIndex);
			// 새 위치를 설정합니다. 회전과 스케일은 변경하지 않습니다.
			BoneTransform.SetLocation(TargetPosition);
			
			// FName a = "thumb_01_r"; 
			// 컴포넌트의 본 변환을 업데이트합니다.
			// HandMesh->SetBoneTransform(BoneIndex, BoneTransform, EBoneSpaces::ComponentSpace);
			/*auto BoneAnim = HandMesh->SkeletalMesh->GetPostProcessAnimBlueprint();
			BoneAnim->getFindSocket(a)->();*/
		}
	}
}

const TMap<int32, FVector>& AAI_Pawn::GetUpdatedBonePositions() const
{
    return BoneIdToPositionMap;
}
