// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPlayerController.generated.h"

class ARTSUnit;
/**
 * 
 */
UCLASS()
class MYRTS_API ARTSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ARTSPlayerController();

	UPROPERTY(BlueprintReadWrite)
	TArray<ARTSUnit*> SelectedUnits;  // Массив выбранных юнитов

	UFUNCTION()
	void SelectUnit(ARTSUnit* Unit);  // Выбрать юнит

	UFUNCTION()
	void MoveSelectedUnits(FVector Location);  // Команда движения
};
