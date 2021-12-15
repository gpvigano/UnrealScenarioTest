// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "EntityStateComponent.h"
#include "GameFramework/Actor.h"
#include "UEUtil.h"


// Sets default values for this component's properties
UEntityStateComponent::UEntityStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEntityStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UEntityStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


FString UEntityStateComponent::GetProperty(FString PropertyName)
{
	FString* propertyValue = Properties.Find(PropertyName);
	if (propertyValue)
	{
		return *propertyValue;
	}
	return FString();
}


bool UEntityStateComponent::CheckProperty(FString PropertyName, FString PropertyValue) const
{
	const FString* foundPropertyValue = Properties.Find(PropertyName);
	if (foundPropertyValue)
	{
		return foundPropertyValue->Equals(PropertyValue);
	}
	return false;

}


USceneComponent* UEntityStateComponent::FindSceneComponent(FString ChildName)
{
	AActor* owner = GetOwner();
	if (owner)
	{
		return UUEUtil::FindSceneComponent(owner, ChildName);
	}
	return nullptr;
}


void UEntityStateComponent::ApplyProperty(FString PropertyName, FString PropertyValue)
{
	Properties.FindOrAdd(PropertyName) = PropertyValue;
	ApplyState();
}
