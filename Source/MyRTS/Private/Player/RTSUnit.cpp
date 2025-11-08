// Fill out your copyright notice in the Description page of Project Settings.



#include "Player/RTSUnit.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

ARTSUnit::ARTSUnit()
{
    bReplicates = true;  // Включаем репликацию
    bIsSelected = false;
}

void ARTSUnit::ServerMoveTo_Implementation(FVector TargetLocation)
{
    // Двигаем юнита к цели (простой AI)
   //AIController->MoveToLocation(TargetLocation);
}

void ARTSUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ARTSUnit, bIsSelected);  // Реплицируем выбор
}