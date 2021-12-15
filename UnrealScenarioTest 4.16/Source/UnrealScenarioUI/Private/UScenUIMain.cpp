// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UScenUIMain.h"



#include "CatalogItemWidget.h"
#include "UScenUtil.h"

#include "Engine/World.h" // GetWorld()
#include "Runtime/UMG/Public/Components/PanelWidget.h"
#include "Runtime/UMG/Public/Components/ComboBoxString.h"


void UUScenUIMain::FillCatalog(UPanelWidget* Panel)
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	if (UnrealScenarioActor)
	{
		if (UnrealScenarioActor && UnrealScenarioActor->CatalogComponent)
		{
			Panel->ClearChildren();
			for (const FCatalogItem& catalogItem : UnrealScenarioActor->CatalogComponent->Items)
			{
				if (!catalogItem.bHidden)
				{
					UCatalogItemWidget* itemWidget = CreateItemWidget();
					if (itemWidget == nullptr)
					{
						return;
					}
					itemWidget->ItemId = catalogItem.ItemId;
					Panel->AddChild(itemWidget);
					itemWidget->UpdateFromCatalogItem();
				}
			}
		}
	}
}


void UUScenUIMain::FillCatalogAndFilter(UPanelWidget* Panel, UComboBoxString* FilterComboBox)
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	if (UnrealScenarioActor && UnrealScenarioActor->CatalogComponent)
	{
		FilterComboBox->ClearOptions();
		FilterComboBox->AddOption(TEXT("All"));
		for (const FCatalogItem& catalogItem : UnrealScenarioActor->CatalogComponent->Items)
		{
			if (!catalogItem.bHidden)
			{
				if (FilterComboBox->FindOptionIndex(catalogItem.Category) < 0)
				{
					FilterComboBox->AddOption(catalogItem.Category);
				}
			}
		}
		FilterComboBox->SetSelectedOption(TEXT("All"));
		FilterCatalog(Panel, TEXT("All"));
	}
}


void UUScenUIMain::FilterCatalog(UPanelWidget* Panel, FString Filter)
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	bool showAll = (Filter == "All");
	if (UnrealScenarioActor && UnrealScenarioActor->CatalogComponent)
	{
		Panel->ClearChildren();
		for (const FCatalogItem& catalogItem : UnrealScenarioActor->CatalogComponent->Items)
		{
			bool showItem = showAll || catalogItem.Category == Filter;
			showItem = showItem && !catalogItem.bHidden;
			if (showItem)
			{
				UCatalogItemWidget* itemWidget = CreateItemWidget();
				if (itemWidget == nullptr)
				{
					return;
				}
				itemWidget->ItemId = catalogItem.ItemId;
				Panel->AddChild(itemWidget);
				itemWidget->UpdateFromCatalogItem();
			}
		}
	}
}



void UUScenUIMain::UpdateOutcomeUI(FString OutcomeString, FString RewardString, FString PerformanceString)
{
	OutcomeText = OutcomeString;
	RewardText = RewardString;
	PerformanceText = PerformanceString;
	UpdateOutcomeText();
}


FText UUScenUIMain::GetOutcomeText()
{
	return FText::FromString(OutcomeText);
}


FText UUScenUIMain::GetRewardText()
{
	return FText::FromString(RewardText);
}


FText UUScenUIMain::GetPerformanceText()
{
	return FText::FromString(PerformanceText);
}


UCatalogItemWidget* UUScenUIMain::CreateItemWidget()
{
	if (CatalogItemWidgetBlueprint == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("CatalogItemWidgetBlueprint not defined"));
		return nullptr;
	}

	UCatalogItemWidget* itemWidget = CreateWidget<UCatalogItemWidget>(GetWorld()->GetFirstPlayerController(), *CatalogItemWidgetBlueprint);

	return itemWidget;
}


