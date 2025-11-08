// RTSCamera.cpp
#include "Camera/RTSCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Player/RTSUnit.h" // Убедись, что путь правильный
#include "GameFramework/PlayerController.h"




ARTSCamera::ARTSCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(SceneComponent);
	SpringArmComponent->TargetArmLength = 2000.0f;
	SpringArmComponent->bDoCollisionTest = true;
	SpringArmComponent->bUsePawnControlRotation = false;  // ← КЛЮЧЕВОЕ!
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;


	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	TargetZoom = 2000.0f;
}

void ARTSCamera::BeginPlay()
{
	Super::BeginPlay();

	TargetLocation = GetActorLocation();
	TargetRotation = FRotator(-50.f, 0.f, 0.f);  // Начальный угол
	TargetZoom = SpringArmComponent->TargetArmLength;
}

void ARTSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// === 1. ПРОВЕРКА СЖАТИЯ SPRINGARM ===
	FVector UnfixedCamPos = SpringArmComponent->GetUnfixedCameraPosition();
	FVector SpringPos = SpringArmComponent->GetComponentLocation();

	float CurrentLength = FVector::Distance(UnfixedCamPos, SpringPos);
	float DesiredLength = SpringArmComponent->TargetArmLength;
	float Ratio = (DesiredLength > 0.f) ? (CurrentLength / DesiredLength) : 1.f;

	bCanRotate = (Ratio >= MinArmLengthRatio);
	// === 2. ДВИЖЕНИЕ X/Y ===
	FVector CurrentLoc = GetActorLocation();
	FVector TargetXY = FVector(TargetLocation.X, TargetLocation.Y, CurrentLoc.Z);
	FVector NewXY = UKismetMathLibrary::VInterpTo(CurrentLoc, TargetXY, DeltaTime, MoveSpeed);
	SetActorLocation(FVector(NewXY.X, NewXY.Y, CurrentLoc.Z));

	// === 3. ЗУМ ===
	float NewArmLength = UKismetMathLibrary::FInterpTo(DesiredLength, TargetZoom, DeltaTime, 15.f);
	SpringArmComponent->TargetArmLength = NewArmLength;

	// === 4. ВРАЩЕНИЕ (ТОЛЬКО ЕСЛИ РАЗРЕШЕНО) ===
	if (bRotationEnabled && bCanRotate)
	{
		FRotator CurrentRot = SpringArmComponent->GetRelativeRotation();
		CurrentRot.Roll = 0.f;
		FRotator DesiredRot = FRotator(TargetRotation.Pitch, TargetRotation.Yaw, 0.f);
		FRotator SmoothRot = UKismetMathLibrary::RInterpTo(CurrentRot, DesiredRot, DeltaTime, RotateSpeed);
		SmoothRot.Roll = 0.f;
		SpringArmComponent->SetRelativeRotation(SmoothRot);
	}

	// === 5. ТРАССИРОВКА ПОД PAWN (чтобы не нырять) ===
	FVector Root = GetActorLocation();
	FVector Start = Root + FVector(0, 0, 1000.f);
	FVector End = Root - FVector(0, 0, 2000.f);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		float TargetZ = Hit.ImpactPoint.Z + GroundOffset;
		float NewZ = FMath::FInterpTo(Root.Z, TargetZ, DeltaTime, 10.f);
		SetActorLocation(FVector(Root.X, Root.Y, NewZ));
	}
}

void ARTSCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem && InputMapping)
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(InputMapping, 0);
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARTSCamera::Move);

		// ← ОТДЕЛЬНЫЕ ОСИ:
		Input->BindAction(LookTurnAction, ETriggerEvent::Triggered, this, &ARTSCamera::RotateHorizontal);
		Input->BindAction(LookUpAction,   ETriggerEvent::Triggered, this, &ARTSCamera::RotateVertical);
		Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ARTSCamera::Zoom);
		Input->BindAction(RotateEnableAction, ETriggerEvent::Started,   this, &ARTSCamera::EnableRotate);
		Input->BindAction(RotateEnableAction, ETriggerEvent::Completed, this, &ARTSCamera::DisableRotate);
		Input->BindAction(FastSpeedAction, ETriggerEvent::Started,   this, &ARTSCamera::FastSpeedEnable);
		Input->BindAction(FastSpeedAction, ETriggerEvent::Completed, this, &ARTSCamera::FastSpeedDisable);
	}
}

// === Движение ===
void ARTSCamera::Move(const FInputActionValue& Value)
{
	FVector2D AxisValue = Value.Get<FVector2D>();
	if (AxisValue.IsNearlyZero()) return;

	FVector Forward = SpringArmComponent->GetForwardVector();
	FVector Right = SpringArmComponent->GetRightVector();

	TargetLocation += Forward * AxisValue.Y * MoveSpeed * CurrentMultiplier;
	TargetLocation += Right * AxisValue.X * MoveSpeed * CurrentMultiplier;

	GetTerrainPosition(TargetLocation);
}

void ARTSCamera::Zoom(const FInputActionValue& InputActionValue)
{
	float ZoomValue = InputActionValue.Get<float>();  // >0 — вверх, <0 — вниз
	TargetZoom -= ZoomValue * ZoomSpeed;   // инвертируем, чтобы "вверх" = приближение

	// Ограничиваем
	TargetZoom = FMath::Clamp(TargetZoom, ZoomMin, ZoomMax);
}

// === Поворот по горизонтали (Mouse X) ===
void ARTSCamera::RotateHorizontal(const FInputActionValue& Value)
{
	if (!bRotationEnabled || !bCanRotate) return;

	float DeltaYaw = Value.Get<float>() * MouseSensitivity;
	TargetRotation.Yaw += DeltaYaw;
}
// === Поворот по вертикали (Mouse Y) ===
void ARTSCamera::RotateVertical(const FInputActionValue& Value)
{
	if (!bRotationEnabled || !bCanRotate) return;

	float DeltaPitch = Value.Get<float>() * MouseSensitivity;
	if (bInvertY) DeltaPitch = -DeltaPitch;

	TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch + DeltaPitch, PitchMin, PitchMax);
}

// === Включение/отключение ===
void ARTSCamera::EnableRotate()
{
	bRotationEnabled = true;

	// СКРЫВАЕМ КУРСОР
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
		PC->bEnableMouseOverEvents = false;

		// Опционально: полностью блокируем ввод в UI
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

void ARTSCamera::DisableRotate()
{
	bRotationEnabled = false;
	bRotationEnabled = false;

	// ПОКАЗЫВАЕМ КУРСОР
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		PC->bEnableMouseOverEvents = true;

		// Возвращаем смешанный режим (игра + UI)
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
	}
}

void ARTSCamera::FastSpeedEnable()
{
	CurrentMultiplier =  FastSpeedMultiplier;
}

void ARTSCamera::FastSpeedDisable()
{
	CurrentMultiplier = 1.f;
}


// === Трассировка на землю ===
void ARTSCamera::GetTerrainPosition(FVector& OutPosition) const
{
	FHitResult Hit;
	FVector Start = OutPosition + FVector(0, 0, 10000.f);
	FVector End = OutPosition - FVector(0, 0, 10000.f);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		OutPosition = Hit.ImpactPoint;
		// НЕ МЕНЯЕМ Z — высота управляется в UpdatePawnHeight
	}
}

void ARTSCamera::UpdatePawnHeight(float DeltaTime)
{
    if (!SpringArmComponent || !CameraComponent) return;

    FVector RootLocation = GetActorLocation();

    // === 1. ТЕКУЩЕЕ СЖАТИЕ SPRINGARM (только снизу) ===
    FVector SpringStart = SpringArmComponent->GetComponentLocation();
    FVector CameraPos = CameraComponent->GetComponentLocation();
    FVector ArmDir = (CameraPos - SpringStart).GetSafeNormal();
    float CurrentLength = (CameraPos - SpringStart).Size();
    float DesiredLength = SpringArmComponent->TargetArmLength;
    float Compression = FMath::Max(0.f, DesiredLength - CurrentLength);

    bool bCompressFromBelow = false;
    if (Compression > 100.f)
    {
        FVector ExpectedPos = SpringStart + ArmDir * DesiredLength;
        FVector Push = CameraPos - ExpectedPos;
        if (Push.Z > 80.f)  // Камера вытолкнута вверх → препятствие СНИЗУ
        {
            bCompressFromBelow = true;
        }
    }

    // === 2. ПРЕДИКТИВНАЯ ТРАССИРОВКА ВПЕРЁД (по направлению камеры) ===
    FVector ForwardDir = SpringArmComponent->GetForwardVector();
    FVector PredictStart = RootLocation + ForwardDir * 800.f;  // 8 метров вперёд
    PredictStart.Z += 500.f;
    FVector PredictEnd = PredictStart - FVector(0, 0, 1500.f);

    FHitResult ForwardHit;
    float ForwardGroundZ = RootLocation.Z;

    if (GetWorld()->LineTraceSingleByChannel(ForwardHit, PredictStart, PredictEnd, ECC_Visibility))
    {
        ForwardGroundZ = ForwardHit.ImpactPoint.Z;
    }

    // === 3. ТРАССИРОВКА ПОД КОРНЕМ ===
    FVector DownStart = RootLocation + FVector(0, 0, 1000.f);
    FVector DownEnd = RootLocation - FVector(0, 0, 2000.f);
    FHitResult DownHit;
    float GroundZ = RootLocation.Z;

    if (GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_Visibility))
    {
        GroundZ = DownHit.ImpactPoint.Z;
    }

    // === 4. ЦЕЛЕВАЯ ВЫСОТА ===
    float TargetZ = GroundZ + GroundOffset;

    // Если впереди склон выше — поднимаем заранее
    if (ForwardGroundZ > GroundZ + 100.f)
    {
        TargetZ = FMath::Max(TargetZ, ForwardGroundZ + GroundOffset - 200.f);  // Немного ниже пика
    }

    // Компенсация сжатия снизу
    if (bCompressFromBelow
    )
    {
        TargetZ += Compression * 0.8f;  // 80% компенсации
    }

    // === 5. ПЛАВНАЯ ИНТЕРПОЛЯЦИЯ ===
    FVector CurrentLoc = GetActorLocation();
    float NewZ = FMath::FInterpTo(CurrentLoc.Z, TargetZ, DeltaTime, 12.f);
    SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, NewZ));
}