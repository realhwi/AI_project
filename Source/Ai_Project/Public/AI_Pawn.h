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

	// 핸드 트래킹 LandmarkId로 본 이름 업데이트 
	UFUNCTION(BlueprintCallable, Category = "Hand Tracking")
	FName GetBoneNameFromLandmarkId(int32 LandmarkId) const;
	void ParseAndApplyHandTrackingData(const FString& ReceivedData);
	// 파이썬으로 전달받은 트래킹 데이터를 언리얼 좌표로 변환 
	FVector ConvertPythonToUnreal(float PixelX, float PixelY, float PixelZ);	
	// 핸드 메시의 위치 업데이트
	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	void UpdateHandMeshPosition(int32 Id, const FVector& NewPosition);
	// 현재 핸드 ID와 본 이름에 해당하는 위치를 얻기 위한 함수 
	/*UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	FVector GetHandPosition(int32 HandId, FName BoneName);*/

	// 손 모델 접근 함수 
	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	USkeletalMeshComponent* GetLeftHandMesh() const { return LeftHandMesh; }

	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	USkeletalMeshComponent* GetRightHandMesh() const { return RightHandMesh; }
	
	// 랜드마크 좌표 저장 맴버 변수 
	TMap<int32, FVector> LandmarkIdToPositionMap;

	FVector GetPositionForLandmarkId(int32 LandmarkId) const;

	FVector ReferencePosition; // 기준점 위치
	bool bHasReference = false; // 기준점이 설정되었는지 여부
public:
	UPROPERTY()
	class ASocketClient* SocketClient;
	
};
