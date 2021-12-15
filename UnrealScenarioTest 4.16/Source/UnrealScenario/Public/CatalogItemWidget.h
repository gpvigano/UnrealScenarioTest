// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnrealScenario.h"
#include "UnrealScenarioActor.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "CatalogItemWidget.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
 Base button widget for Unreal Scenario catalog items.
 A blueprint must be derived from this widget,
 defining the item button and its appearance.
 */
class UNREALSCENARIO_API UCatalogItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
		/**
		Unique identifier of the item in the catalog.
		*/
		FString ItemId;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
		/**
		Update the widget appearance according to the item data.
		*/
		void UpdateFromCatalogItem();

	UFUNCTION(BlueprintCallable, Category = "Catalog")
		/**
		Add a new element to the scene according to the defined item data.
		*/
		void CreateFromCatalogItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		/**
		Image component used to display an icon on the catalog item button.
		*/
	UImage* ItemIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		/**
		Text component used to display a name on the catalog item button.
		*/
	UTextBlock* ItemText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		/**
		Button component used to add a new element to the scene.
		*/
	UButton* ItemButton = nullptr;

protected:

	UPROPERTY()
	AUnrealScenarioActor* UnrealScenarioActor;

	UFUNCTION()
	void Init();

	//TSharedRef<SWidget> RebuildWidget() override;
};
