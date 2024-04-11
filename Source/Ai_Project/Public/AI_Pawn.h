// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" 
#include "AI_Pawn.generated.h"

UCLASS()
class AI_PROJECT_API AAI_Pawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Pawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 입력 매핑 컨텍스트 + 액션 
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = Input , meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* IMC_AI;
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = Input , meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Look;
	// 카메라 회전 기능 
	void RotateCamera(const FInputActionValue& Value);
	
	// 생성자 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USkeletalMeshComponent* RightHandMesh;

    // 본 ID에 따라 본 이름 가져오기
	UFUNCTION(BlueprintCallable, Category = "Hand Tracking")
	FName GetBoneNameFromLandmarkId(int32 LandmarkId, const FString& HandType) const;
	// 웹캠 데이터 파싱 및 핸드 트래킹 데이터 적용
	void ParseAndApplyHandTrackingData(const FString& ReceivedData);
    // 웹캠 데이터로부터 언리얼 엔진 좌표계로 변환
	FVector ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ);	
    // 웹캠 데이터를 기반으로 핸드 메시 위치 업데이트
	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	void UpdateHandMeshPosition(const FString& HandType, const FVector& NewPosition, const FRotator& NewRotation);
	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	void UpdateBonePositions(const TMap<int32, FVector>& BoneIdToPositionMap, const FString& HandType);
	
	// 랜드마크 좌표 저장 맴버 변수 
	TMap<int32, FVector> LandmarkIdToPositionMap;
	
	FVector ReferencePosition; // 기준점 위치
	bool bHasReference = false; // 기준점이 설정되었는지 여부
	
	UPROPERTY()
	class ASocketClient* SocketClient;	
	FVector InitialCameraLocation;     // 초기 카메라 위치
	FVector HandMeshOffsetFromCamera; // 카메라로부터 핸드 메시까지의 상대적 거리

	FVector InitialRightHandLocation; // 오른손 초기 위치
	FRotator InitialRightHandRotation; // 오른손의 초기 회전
	FVector InitialLeftHandLocation; // 왼손의 초기 위치
	FRotator InitialLeftHandRotation; // 왼손의 초기 회전
	bool bInitialHandPositionSet = false; // 초기 손 위치가 설정되었는지 나타내는 플래그

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandTracking")
	float RotationSpeed = 0.1f; // 적절한 기본값 설정

};
