// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UnrealScenarioTestActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"
#include "EntityStateComponent.h"

#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/GameFramework/HUD.h"
#include "Components/TextRenderComponent.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Blueprint/WidgetTree.h"

#include "UScenUIBase.h"
#include "UScenUIMain.h"
#include "UScenUIConfig.h"

#include "discenfw/util/MessageLog.h"
#include "discenfw/scen/ScenarioManager.h"
#include "discenfw/DigitalScenarioFramework.h"


AUnrealScenarioTestActor::AUnrealScenarioTestActor()
{
	if (XpFileName.IsEmpty())
	{
		XpFileName = "test_xp.json";
	}
	DontHighlightTag = TEXT("Icon");

	InitElementSpawnSlot = FVector::ZeroVector;
}

void AUnrealScenarioTestActor::MoveActorToNextSlotLocation(AActor* ElementActor)
{
	Super::MoveActorToNextSlotLocation(ElementActor);

	// Compute the next location
	ElementSpawnSlot.Y += 150.0f;
	if (ElementSpawnSlot.Y > 500.0f)
	{
		ElementSpawnSlot.X -= 50.0f;
		ElementSpawnSlot.Y = -ElementSpawnSlot.Y;
		ElementSpawnSlot.Z -= 50.0f;
	}
	if (ElementSpawnSlot.X < -150.0f)
	{
		ElementSpawnSlot = InitElementSpawnSlot;
	}
}


void AUnrealScenarioTestActor::ClearScene()
{
	Super::ClearScene();
	ElementSpawnSlot = InitElementSpawnSlot;
}


void AUnrealScenarioTestActor::BeginPlay()
{
	Super::BeginPlay();

	ElementSpawnSlot = InitElementSpawnSlot;
	FString tooltipName = TEXT("TooltipLabel");
	if (!TooltipLabel && AssetIndexComponent->ActorBlueprints.Contains(tooltipName))
	{
		TooltipLabel = AssetIndexComponent->CreateActorFromAsset(tooltipName);
	}
	if (!TooltipLabel && AssetIndexComponent->SceneActors.Contains(tooltipName))
	{
		TooltipLabel = AssetIndexComponent->CreateActorFromSceneActor(tooltipName);
	}

	if (TooltipLabel)
	{
		UActorComponent* tooltipActorComponent = TooltipLabel->FindComponentByClass(UWidgetComponent::StaticClass());
		if (tooltipActorComponent)
		{
			UWidgetComponent* labelWidget = CastChecked<UWidgetComponent>(tooltipActorComponent);
			if (labelWidget && labelWidget->GetUserWidgetObject())
			{
				UUserWidget* uWidget = labelWidget->GetUserWidgetObject();
				TArray<UWidget*> widgets;
				uWidget->WidgetTree->GetAllWidgets(widgets);
				for (UWidget* widget : widgets)
				{
					LabelTextBlock = Cast<UTextBlock>(widget);
					if (LabelTextBlock != nullptr)
					{
						break;
					}
				}
			}
		}
		else
		{
			TArray<UActorComponent*> actorComponents;
			UUEUtil::GetActorComponents(TooltipLabel, actorComponents, UTextRenderComponent::StaticClass());
			LabelTextRenderers.Empty();
			for (UActorComponent* actorComponent : actorComponents)
			{
				UTextRenderComponent* labelTextRender = CastChecked<UTextRenderComponent>(actorComponent);
				LabelTextRenderers.Add(labelTextRender);
			}
		}
		// hide the actor used to show tooltips
		UUEUtil::SetActorActive(TooltipLabel, false);
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Tooltip Label not defined"));
	}
}


void AUnrealScenarioTestActor::OnCancel()
{
	Super::OnCancel();
	UUEUtil::SetActorActive(TooltipLabel, false);
	using namespace discenfw;

	if (DiScenFw()->GetLastActionOutcome(AgentName).CompletedEpisode)
	{
		NewEpisode();
	}
}




#if WITH_EDITOR

void AUnrealScenarioTestActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!TooltipLabel
		&& AssetIndexComponent->IsSceneActorDefined(TEXT("TooltipLabel"))
		&& !AssetIndexComponent->IsActorBlueprintDefined(TEXT("TooltipLabel")))
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("TooltipLabel not defined. Define it in Asset Index Component"));
	}
}

#endif


void AUnrealScenarioTestActor::SetLabelText(const FString& LabelText, const FVector& LabelLocation)
{
	if (TooltipLabel)
	{
		UUEUtil::SetActorActive(TooltipLabel, true);
		FText text = FText::FromString(LabelText);
		if (LabelTextBlock)
		{
			LabelTextBlock->SetText(text);
		}
		else
		{
			for (UTextRenderComponent* labelTextRender : LabelTextRenderers)
			{
				labelTextRender->SetText(text);
			}
		}
		TooltipLabel->SetActorLocation(LabelLocation);
	}
}
