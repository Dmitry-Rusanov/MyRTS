// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/RTSGameMode.h"

ARTSGameMode::ARTSGameMode()
{
    // Базовые настройки для мультиплеера
    bUseSeamlessTravel = true;  // Плавный переход между картами
}

void ARTSGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    //TODO: Здесь позже добавим спавн юнитов для нового игрока
    UE_LOG(LogTemp, Warning, TEXT("Игрок %s зашёл!"), *NewPlayer->GetName());
}
