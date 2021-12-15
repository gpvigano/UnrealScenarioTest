// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "CatalogItemWidget.h"
#include "UScenUtil.h"

#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Blueprint/WidgetTree.h"
#include "Runtime/UMG/Public/Components/CanvasPanel.h"


void UCatalogItemWidget::UpdateFromCatalogItem()
{
	Init();
	if (UnrealScenarioActor && UnrealScenarioActor->CatalogComponent)
	{
		bool hasFirstOne = false;
		for (const FCatalogItem& catalogItem : UnrealScenarioActor->CatalogComponent->Items)
		{
			if (catalogItem.bFirstOne)
			{
				hasFirstOne = true;
				break;
			}
		}
		for (const FCatalogItem& catalogItem : UnrealScenarioActor->CatalogComponent->Items)
		{
			if (catalogItem.ItemId == ItemId)
			{
				if (ItemIcon)
				{
					UTexture2D* texture = UnrealScenarioActor->AssetIndexComponent->GetTextureById(catalogItem.Icon);
					ItemIcon->SetBrushFromTexture(texture);
				}
				if (ItemText)
				{
					FString displayName = catalogItem.DisplayName;
					// if there is no icon nor tooltip, then show at least the id
					if (displayName.IsEmpty() && catalogItem.Icon.IsEmpty())
					{
						displayName = catalogItem.ItemId;
					}
					ItemText->SetText(FText::FromString(displayName));
				}
				if (ItemButton)
				{
					ItemButton->SetToolTipText(FText::FromString(catalogItem.ToolTip));

					// TODO: these rules should be controlled by the digital system
					// maybe with a method (cyberSystem)->CanBeAdded()

					bool bEnabled = true;
					bool isEmpty = UnrealScenarioActor->IsScenarioEmpty();
					if(catalogItem.bFirstOne)
					{
						bEnabled = isEmpty;
					}
					else
					{
						bEnabled = !hasFirstOne || !isEmpty;
					}
					if(bEnabled && catalogItem.AllowedInstances>=0)
					{
						bEnabled = UnrealScenarioActor->CountEntitiesByCategory(catalogItem.Category)<=catalogItem.AllowedInstances;
					}

					ItemButton->SetIsEnabled(bEnabled);
				}
				break;
			}
		}
	}
}


void UCatalogItemWidget::CreateFromCatalogItem()
{
	Init();
	UnrealScenarioActor->AddElementFromCatalog(ItemId, true);
}

/*
TSharedRef<SWidget> UCatalogItemWidget::RebuildWidget()
{
	if ( !bInitialized )
	{
		Initialize();
	}

	// Create a CanvasPanel to use as the default root widget
	if (WidgetTree->RootWidget == nullptr)
	{
		UWidget* root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		WidgetTree->RootWidget = root;
		UCanvasPanel* rootPanel = CastChecked<UCanvasPanel>(root);
		UWidget* buttonWidget = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		ItemButton = CastChecked<UButton>(buttonWidget);
		rootPanel->AddChildToCanvas(ItemButton);
		UWidget* imageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		UImage* ItemIcon = CastChecked<UImage>(imageWidget);
		ItemButton->AddChild(ItemIcon);
		//ItemButton->OnClicked = CreateFromCatalogItem;
	}

	return Super::RebuildWidget();
}
*/

void UCatalogItemWidget::Init()
{
	if (UnrealScenarioActor == nullptr)
	{
		UnrealScenarioActor = UUScenUtil::FindUnrealScenario(GetWorld());

		TArray<UWidget*> widgets;
		WidgetTree->GetAllWidgets(widgets);
		for (UWidget* widget : widgets)
		{
			if (ItemText == nullptr)
			{
				ItemText = Cast<UTextBlock>(widget);
			}
			if (ItemButton == nullptr)
			{
				ItemButton = Cast<UButton>(widget);
				if (ItemButton)
				{
					for (int32 i = 0; i < ItemButton->GetChildrenCount(); i++)
					{
						UWidget* child = ItemButton->GetChildAt(i);
						if (ItemIcon == nullptr)
						{
							ItemIcon = Cast<UImage>(child);
						}
						if (ItemText == nullptr)
						{
							ItemText = Cast<UTextBlock>(child);
						}
					}
				}
			}
		}
	}
}

