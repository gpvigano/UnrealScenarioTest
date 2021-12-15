// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "CatalogComponent.h"
#include "UnrealScenarioActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"

#include "discenfw/DigitalScenarioFramework.h"
#include "discenfw/scen/Catalog.h"


// Sets default values for this component's properties
UCatalogComponent::UCatalogComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;// true;

	// ...
}


// Called when the game starts
void UCatalogComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UCatalogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UCatalogComponent::LoadCatalog()
{
	AUnrealScenarioActor* unrealScenarioActor = UUScenUtil::FindUnrealScenario(GetWorld());
	using namespace discenfw;
	std::string catalogFile = UUEUtil::ToStdString(unrealScenarioActor->ScenarioDir() + unrealScenarioActor->CatalogComponent->FileName);
	std::string catalogName = UUEUtil::ToStdString(unrealScenarioActor->CatalogComponent->Name);
	const std::shared_ptr<Catalog> catalog = DiScenFw()->LoadCatalog(catalogName, catalogFile);
	if (!catalog)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to load catalog from ") + unrealScenarioActor->CatalogComponent->FileName, false, true);
		return;
	}
	//catalog->Items.front().Badge.Description = "data protection test";
	unrealScenarioActor->CatalogComponent->Items.Empty();
	for (const CatalogItem& item : catalog->GetItems())
	{
		FCatalogItem newItem;
		newItem.ItemId = UUEUtil::ToFString(item.Badge.Identifier);
		newItem.TypeId = UUEUtil::ToFString(item.Badge.Type);
		newItem.Category = UUEUtil::ToFString(item.Badge.Category);
		newItem.DisplayName = UUEUtil::ToFString(item.Badge.DisplayName);
		newItem.Icon = UUEUtil::ToFString(item.Badge.Icon);
		newItem.ToolTip = UUEUtil::ToFString(item.Badge.Description);
		newItem.AssetSource = (EAssetSource)item.Asset.Source;
		newItem.AssetId = UUEUtil::ToFString(item.Asset.Uri);
		newItem.AssetType = UUEUtil::ToFString(item.Asset.AssetType);
		newItem.bFirstOne = item.FirstOne;
		newItem.AllowedInstances = item.AllowedInstances;
		newItem.bHidden = item.Hidden;
		unrealScenarioActor->CatalogComponent->Items.Add(newItem);
	}

	unrealScenarioActor->CatalogComponent->CategoryAbbreviations.Empty();
	for (const auto& typeTag : catalog->GetCategoryAbbreviations())
	{
		FString catStr = UUEUtil::ToFString(typeTag.first);
		FString abbrevStr = UUEUtil::ToFString(typeTag.second);
		unrealScenarioActor->CatalogComponent->CategoryAbbreviations.Add(catStr, abbrevStr);
	}
}


void UCatalogComponent::SaveCatalog()
{
	AUnrealScenarioActor* unrealScenarioActor = UUScenUtil::FindUnrealScenario(GetWorld());
	using namespace discenfw;
	std::string catalogFile = UUEUtil::ToStdString(unrealScenarioActor->ScenarioDir() + unrealScenarioActor->CatalogComponent->FileName);
	std::string catalogName = UUEUtil::ToStdString(unrealScenarioActor->CatalogComponent->Name);
	DiScenFw()->CreateCatalog(catalogName);
	for (const FCatalogItem& item : unrealScenarioActor->CatalogComponent->Items)
	{
		AssetDefinition::SourceType source = (AssetDefinition::SourceType)item.AssetSource;
		if (source == AssetDefinition::UNDEFINED)
		{
			bool bBpDef = unrealScenarioActor->AssetIndexComponent->IsActorBlueprintDefined(item.ItemId);
			source = bBpDef ? AssetDefinition::PROJECT : AssetDefinition::SCENE;
		}

		bool hidden = item.bHidden;
		CatalogItem catalogItem;
		catalogItem.Badge.Identifier = UUEUtil::ToStdString(item.ItemId);
		catalogItem.Badge.Type = UUEUtil::ToStdString(item.TypeId);
		catalogItem.Badge.Category = UUEUtil::ToStdString(item.Category);
		catalogItem.Badge.DisplayName = UUEUtil::ToStdString(item.DisplayName);
		catalogItem.Badge.Icon = UUEUtil::ToStdString(item.Icon);
		catalogItem.Badge.Description = UUEUtil::ToStdString(item.ToolTip);
		catalogItem.Hidden = item.bHidden;
		std::string typeStr = UUEUtil::ToStdString(item.AssetType);
		std::string idStr = UUEUtil::ToStdString(item.AssetId);
		catalogItem.Asset = { idStr, typeStr, source };
		catalogItem.FirstOne = item.bFirstOne;
		catalogItem.AllowedInstances = item.AllowedInstances;
		DiScenFw()->AddCatalogItem(catalogName, catalogItem);
	}

	for (const auto& abbreviation : unrealScenarioActor->CatalogComponent->CategoryAbbreviations)
	{
		DiScenFw()->SetCatalogCategoryAbbreviation(catalogName,
			UUEUtil::ToStdString(abbreviation.Key),
			UUEUtil::ToStdString(abbreviation.Value)
			);
	}

	DiScenFw()->SaveCatalog(catalogName, catalogFile);
}

