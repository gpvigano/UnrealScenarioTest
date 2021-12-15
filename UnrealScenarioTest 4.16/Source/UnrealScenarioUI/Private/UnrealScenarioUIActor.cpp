// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UnrealScenarioUIActor.h"

#include "UScenUtil.h"

#include "UScenUIMain.h"
#include "UScenUIConfig.h"



AUnrealScenarioUIActor::AUnrealScenarioUIActor()
{
	DontHighlightTag = TEXT("Icon");
}


void AUnrealScenarioUIActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();

	// toggle UI
	if (playerController->WasInputKeyJustPressed(EKeys::Tab))
	{
		if (!ConfigUiWidget->IsInViewport())
		{
			if (MainUiWidget->IsInViewport())
			{
				MainUiWidget->RemoveFromViewport();
			}
			else
			{
				MainUiWidget->AddToViewport();
				MainUiWidget->UpdateUI();
			}
		}
		return;
	}
}


void AUnrealScenarioUIActor::NewEpisode()
{
	Super::NewEpisode();
	UpdateOutcomeText();
}


void AUnrealScenarioUIActor::UpdateHints()
{
	// Reset all icons (scene components with "Icon" tag)
	for (const AActor* actor : EntityActors)
	{
		TArray<UActorComponent*> icons = actor->GetComponentsByTag(USceneComponent::StaticClass(), TEXT("Icon"));
		for (UActorComponent* actorComp : icons)
		{
			UPrimitiveComponent* icon = CastChecked<UPrimitiveComponent>(actorComp);
			icon->bHiddenInGame = true;
			icon->MarkRenderStateDirty();
		}
	}
}


#if WITH_EDITOR

void AUnrealScenarioUIActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (IconMaterialForbidden == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Icon Material Forbidden not defined"));
	}
	if (IconMaterialSuggested == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Icon Material Suggested not defined"));
	}
	if (MainUiBlueprint == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Main UI Blueprint not defined"));
	}
	if (ConfigurationUiBlueprint == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Configuration UI Blueprint not defined"));
	}
}

#endif



void AUnrealScenarioUIActor::ToggleConfigurationUI(bool bEnabled)
{
	if (bEnabled && !ConfigUiWidget->IsInViewport())
	{
		MainUiWidget->RemoveFromViewport();
		ConfigUiWidget->AddToViewport();
		ConfigUiWidget->UpdateUI();
	}
	if (!bEnabled && ConfigUiWidget->IsInViewport())
	{
		ConfigUiWidget->RemoveFromViewport();
		MainUiWidget->AddToViewport();
		MainUiWidget->UpdateUI();
	}
}


void AUnrealScenarioUIActor::BeginPlay()
{
	Super::BeginPlay();

	// add the User interface to the viewport

	MainUiWidget = CreateWidget<UUScenUIMain>(GetWorld()->GetFirstPlayerController(),*MainUiBlueprint);
	ConfigUiWidget = CreateWidget<UUScenUIConfig>(GetWorld()->GetFirstPlayerController(),*ConfigurationUiBlueprint);

	MainUiWidget->AddToViewport();
	MainUiWidget->UpdateUI();
}


void AUnrealScenarioUIActor::OnScenarioLoaded()
{
	Super::OnScenarioLoaded();
	UpdateMainUI();
}


void AUnrealScenarioUIActor::OnEntityAdded()
{
	Super::OnEntityAdded();
	UpdateMainUI();
}


void AUnrealScenarioUIActor::OnEntityRemoved()
{
	Super::OnEntityRemoved();
	UpdateMainUI();
}


void AUnrealScenarioUIActor::OnActionDone(bool bCompletedEpisode, bool bUpdateScene)
{
	bool bPrevAutoTraining = bAutoTrainingEnabled;
	Super::OnActionDone(bCompletedEpisode, bUpdateScene);
	bool bAutoTrainingStopped = bPrevAutoTraining && !bAutoTrainingEnabled;
	UpdateOutcomeText();
	if (MainUiWidget && MainUiWidget->IsInViewport())
	{
		MainUiWidget->UpdateStats();
		if ((bAutoTrainingStopOnSuccess || bStopAutoTrainingOnCompletion) && bCompletedEpisode)
		{
			// this is expected to be enabled again when needed
			bStopAutoTrainingOnCompletion = false;
			MainUiWidget->UpdateAutoTrainingEnabledCheckBox();
		}
	}
}


void AUnrealScenarioUIActor::UpdateMainUI()
{
	if (MainUiWidget && MainUiWidget->IsInViewport())
	{
		MainUiWidget->UpdateUI();
	}
}


void AUnrealScenarioUIActor::UpdateOutcomeText()
{
	if (MainUiWidget && MainUiWidget->IsInViewport())
	{
		using namespace discenfw;
		using namespace discenfw::xp;
		const ActionOutcome& outcome = DiScenFw()->GetLastActionOutcome(AgentName);
		FString outcomeText;
		FString rewardText;
		FString performanceText;
		if (outcome.Error == ActionError::NO_ERROR)
		{
			if (outcome.Stuck)
			{
				outcomeText = TEXT("Stuck");
				performanceText = TEXT("n/a");
			}
			else switch (outcome.Result)
			{
			case ActionResult::SUCCEEDED:
				outcomeText = TEXT("Succeeded");
				performanceText = FString::FromInt(outcome.Performance);
				break;
			case ActionResult::FAILED:
				outcomeText = TEXT("Failed");
				performanceText = TEXT("n/a");
				break;
			case ActionResult::DEADLOCK:
				outcomeText = TEXT("Deadlock");
				performanceText = TEXT("n/a");
				break;

			default:
				outcomeText = TEXT("(in progress)");
				performanceText = TEXT("...");
				break;
			}
			rewardText = FString::FromInt(outcome.Reward);
		}
		else
		{
			rewardText = TEXT("-");
			performanceText = TEXT("-");
			switch (outcome.Error)
			{
			case ActionError::AGENT_NOT_FOUND:
				outcomeText = TEXT("Error (agent not found)");
				break;
			case ActionError::NO_ACTION_FOUND:
				outcomeText = TEXT("-");
				break;
			default:
				outcomeText = TEXT("Unknown error");
				break;
			}
		}
		MainUiWidget->UpdateOutcomeUI(outcomeText, rewardText, performanceText);
	}
}

