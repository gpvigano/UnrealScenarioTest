// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ViewportUiDisplayComponent.h"


// Sets default values for this component's properties
UViewportUiDisplayComponent::UViewportUiDisplayComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UViewportUiDisplayComponent::BeginPlay()
{
	Super::BeginPlay();

	UiWidget = CreateWidget<UUScenUIBase>(GetWorld()->GetFirstPlayerController(),*UiWidgetBlueprint);

	if (bStartWithUiVisible)
	{
		UiWidget->AddToViewport();
		UiWidget->UpdateUI();
	}
	
}


// Called every frame
void UViewportUiDisplayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	APlayerController* playerController = GetWorld()->GetFirstPlayerController();

	// toggle UI
	if (UiWidget && playerController->WasInputKeyJustPressed(EKeys::Tab))
	{
		if (UiWidget->IsInViewport())
		{
			UiWidget->RemoveFromViewport();
		}
		else
		{
			UiWidget->AddToViewport();
			UiWidget->UpdateUI();
		}
	}
}

