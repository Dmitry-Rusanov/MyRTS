// RTSCamera.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "RTSCamera.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class ARTSUnit; // Добавляем для следования

UCLASS()
class MYRTS_API ARTSCamera : public APawn
{
	GENERATED_BODY()

public:
	ARTSCamera();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// === Движение ===
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	// === Zoom ===
	void Zoom(const FInputActionValue& InputActionValue);
	
	// === Поворот (отдельные оси) ===
	UFUNCTION()
	void RotateHorizontal(const FInputActionValue& Value);  // Mouse X → Yaw

	UFUNCTION()
	void RotateVertical(const FInputActionValue& Value);    // Mouse Y → Pitch

	// === Включение/выключение поворота ===
	UFUNCTION()
	void EnableRotate();

	UFUNCTION()
	void DisableRotate();

	void FastSpeedEnable();
	void FastSpeedDisable();
	// === Утилита: позиция под курсором (опционально) ===
	UFUNCTION()
	void GetTerrainPosition(FVector& OutPosition) const;

	UFUNCTION()
	void UpdatePawnHeight(float DeltaTime);

private:
	// === Компоненты ===
	UPROPERTY(VisibleAnywhere, Category = "C++|Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, Category = "C++|Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;



	UPROPERTY(VisibleAnywhere, Category = "C++|Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	// === Input ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookTurnAction;     // Mouse X

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookUpAction;       // Mouse Y

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateEnableAction; // Middle Mouse Button

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++|EnhancedInput", meta = (AllowPrivateAccess = "true"))
	UInputAction* FastSpeedAction; // LShift Button

	// === Настройки камеры ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float MoveSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float FastSpeedMultiplier = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float RotateSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	bool bInvertY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float PitchMin = -80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float PitchMax = -10.f;


	// === Настройки зума ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomMin = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomMax = 4000.f;

	UPROPERTY()
	float TargetZoom = 2000.f;



	
	// === Защита от врезания === //
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float GroundOffset = 300.f;        // Высота Pawn над землёй

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float MaxSpringArmCompression = 800.f;  // Если SpringArm сжат больше чем на это → поднимаем Pawn

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++|Camera", meta = (AllowPrivateAccess = "true"))
	float MinArmLengthRatio = 0.5f;  // Если SpringArm сжат < 50% → блокируем вращение

	UPROPERTY()
	bool bCanRotate = true;  // Разрешение вращения
	
	
	// === Состояние ===
	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	FRotator TargetRotation;

	UPROPERTY()
	bool bRotationEnabled = false;

	UPROPERTY()
    float CurrentMultiplier = 1.f;

	// Edge Scrolling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Edge Scrolling", meta=(AllowPrivateAccess = "true"))
	bool bEdgeScrollingEnabled = true;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Edge Scrolling", meta = (ClampMin = "0", ClampMax = "500", AllowPrivateAccess = "true"))
	float EdgeScrollBorder = 20.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Edge Scrolling", meta = (AllowPrivateAccess = "true"))
	float EdgeScrollSpeed = 1000.0f;

private:
	void HandleEdgeScrolling(float DeltaTime);
};