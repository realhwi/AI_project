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

	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = Input , meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* IMC_AI;

	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = Input , meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings | Player")
	class USkeletalMeshComponent* RightHandMesh;

	void RotateCamera(const FInputActionValue& Value);
	FName GetBoneNameFromLandmarkId(int32 LandmarkId) const;
	void ParseAndApplyHandTrackingData(const FString& ReceivedData);
	
	FVector ConvertPythonToUnreal(float PythonX, float PythonY);
	float ConvertPythonValueToUnreal(float PythonValue, float PythonMin, float PythonMax, float UnrealMin, float UnrealMax);

};
