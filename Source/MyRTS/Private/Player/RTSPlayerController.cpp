

// Класс контроллера игрока для стратегии в реальном времени (RTS)
#include "Player/RTSPlayerController.h"
#include "Engine/World.h"  // Для работы с игровым миром
#include "Player/RTSUnit.h"  // Для работы с юнитами

// Конструктор контроллера игрока
ARTSPlayerController::ARTSPlayerController()
{
	bReplicates = true;  // Включаем репликацию для сетевой игры
}

// Метод выделения юнита
void ARTSPlayerController::SelectUnit(ARTSUnit* Unit)
{
	if (Unit)  // Проверяем, что юнит существует
	{
		Unit->bIsSelected = true;  // Устанавливаем флаг выделения
		SelectedUnits.Add(Unit);  // Добавляем юнита в список выделенных
	}
}

// Метод перемещения выделенных юнитов в указанную точку
void ARTSPlayerController::MoveSelectedUnits(FVector Location)
{
	for (ARTSUnit* Unit : SelectedUnits)  // Проходим по всем выделенным юнитам
	{
		if (Unit && HasAuthority())  // Проверяем существование юнита и права сервера
		{
			Unit->ServerMoveTo(Location);  // Отправляем команду на перемещение на сервер
		}
	}
}

