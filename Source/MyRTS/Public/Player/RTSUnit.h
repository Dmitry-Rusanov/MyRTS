// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTSUnit.generated.h"

UCLASS(Abstract)
class MYRTS_API ARTSUnit : public ACharacter
{
	GENERATED_BODY()

public:
    ARTSUnit();  // Конструктор

    UPROPERTY(Replicated, BlueprintReadWrite)  // Реплицируется в сеть
    bool bIsSelected;  // Выбран ли юнит

    UFUNCTION(Server, Reliable)  // Серверная функция для движения
    void ServerMoveTo(FVector TargetLocation);  // Движение к точке

	//Получить свойства, которые должны реплицироваться в течение всего времени жизни этого объекта
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

