// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "UnrealScenario.h"
#include "Logger.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include <string>



#include "UEUtil.generated.h"

UCLASS()
/**
 * Unreal Engine Blueprint Utility Library
 */
class UNREALSCENARIO_API UUEUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	Global logger.
	*/
	static Logger Log;


	/// @name String/name conversion utility.
	///@{

	static FString ToFString(FName Name)
	{
		return Name.ToString();
	}

	static FName ToFName(FString Str)
	{
		return FName(*Str);
	}

	static std::string ToStdString(FName Str)
	{
		return ToStdString(Str.ToString());
	}

	static FName ToFName(const std::string& Str)
	{
		return ToFName(ToFString(Str));
	}

	static std::string ToStdString(FString Str)
	{
		return std::string(TCHAR_TO_UTF8(*Str));
	}

	static FString ToFString(const std::string& Str)
	{
		return Str.empty() ? FString() : FString(Str.c_str());
	}

	///@}


	/// @name Get a list of components of the given actor.
	///@{

	static void GetActorComponents(AActor* Actor, TArray<UActorComponent*>& ActorComponents, TSubclassOf<UActorComponent> ActorComponentClass);

	static void GetPrimitiveComponents(AActor* Actor, TArray<UPrimitiveComponent*>& PrimComponents);

	static void GetMeshComponents(AActor* Actor, TArray<UMeshComponent*>& MeshComponents);

	static void GetSceneComponents(AActor* Actor, TArray<USceneComponent*>& SceneComponents);

	///@}

	/**
	Find a scene component with the given name in the given actor.
	*/
	static USceneComponent* FindSceneComponent(AActor* Actor, FString Name);

	/**
	Set an actor visibility, even recursively.
	@param ExistingActor The actor to be shown/hidden.
	@param Visible Visibility (true) or invisibility (false).
	@param AffectChildren Change also the visibility of attached actors.
	*/
	static bool SetActorVisible(AActor* ExistingActor, bool Visible, bool AffectChildren = true);

	/**
	Set an actor active in the scene, even recursively.
	@param ExistingActor The actor to be shown/hidden.
	@param Active Active (true) or inactive (false).
	@param AffectChildren Change also the visibility of attached actors.
	*/
	static bool SetActorActive(AActor* ExistingActor, bool Active, bool AffectChildren = true);

	/// @name Data exchange with blueprints.
	///@{

	static bool SetActorVectorProperty(AActor* ExistingActor, FName PropertyName, FVector VectorValue );

	static bool SetActorSceneCompProperty(AActor* ExistingActor, FName PropertyName, USceneComponent* SceneComponent );

	static USceneComponent* GetActorSceneCompProperty(AActor* ExistingActor, FName PropertyName);

	static bool CallActorFunctionWithArguments(AActor* ExistingActor, const FString Command);
	///@}

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Create an actor in the given world from the given class, optionally attached to another actor.
		*/
		static AActor* CreateActorOfClass(UWorld* World, TSubclassOf<AActor> ActorClass, FName NewName, AActor* ParentActor);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Create a clone of an actor, optionally attached to another actor.
		*/
		static AActor* CreateCloneOfActor(AActor* ExistingActor, FString NewName, AActor* ParentActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Create a clone of an actor, optionally attached to another actor, and set its location and rtation.
		*/
		static AActor* CreateCloneOfActorAndPlace(AActor* ExistingActor, FString NewName, FVector SpawnLocation, FRotator SpawnRotation, AActor* ParentActor = nullptr, bool Relative = true);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Check if an object with the given name exists in the given level.
		*/
		static bool ExistingObject(ULevel* OuterLevel, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Get the full path given a relative path and a file name, create it if it does not exist.
		@return Returning true if the path finally exists (stored in OutAbsoluteFilePath), false on error.
		*/
		static bool GetOrCreatePath(const FString& DirectoryPath, const FString& FileName, FString& OutAbsoluteFilePath);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Check if the given file path exists.
		*/
		static bool FileExists(const FString& AbsoluteFilePath);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Save a text to a file.
		*/
		static bool SaveTextFile(const FString& SaveDirectory, const FString& FileName, const FString& TextToSave, bool AllowOverwriting = false);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Load a text from a file.
		*/
		static bool LoadTextFile(const FString& DirectoryPath, const FString& FileName, FString& TextToLoad);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Set an actor active in the scene, even recursively.
		@param ExistingActor The actor to be shown/hidden.
		@param Active Active (true) or inactive (false).
		@param AffectRendering Enable or disable the visibility.
		@param AffectCollision Enable or disable the visibility.
		@param AffectTick Enable or disable updates (tick).
		@param AffectChildren Change also the visibility of attached actors.
		*/
		static bool SetActorActive(AActor* ExistingActor, bool Active, bool AffectRendering, bool AffectCollision, bool AffectTick, bool AffectChildren = true);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Check if an actor is visible or hidden.
		*/
		static bool GetActorVisible(AActor* ExistingActor);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Destroy an array of actors and their children, clearing overlaps.
		*/
		static void DestroyActorsArray(TArray<AActor*>& Actors, bool ClearOverlaps = true, bool DestroyChildren = true);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Pick the actor under the cursor (nullptr if not found) and get the distance from the cursor.
		*/
		static AActor* PickActorUnderCursor(APlayerController* PlayerController, float& OutDistance);


	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Get the distance of an actor from the eyes view point.
		*/
		static float GetDistanceFromEyesViewPoint(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Set the distance of an actor from the eyes view point.
		*/
		static void SetDistanceFromEyesViewPoint(USceneComponent* SceneComponent, float Distance);


	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Drag an actor following the cursor position and keeping the given distance from the eyes view point.
		*/
		static void MouseDragActor(AActor* DraggedActor, float Distance);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Drag a location following the cursor position and keeping the given distance from the eyes view point.
		*/
		static bool MouseDragLocation(APlayerController* PlayerController, float Distance, FVector& Location);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Rotate an actor along the world vertical axis by the given degrees.
		*/
		static void RotateAlongVerticalAxis(AActor* Actor, float Degrees);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Rotate a scene component following the player rotation or its view point rotation (default).
		*/
		static void RotateWithPlayer(APlayerController* PlayerController, USceneComponent* SceneComponent, bool AlignWithViewPoint = true);


	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Capturea screenshot ffrom the current viewport and save it to the given path, optionally (default) appending a suffix for multiple captures.
		*/
		static bool SaveScreenshot(const FString& DirectoryPath, const FString& FileName, bool AppendSuffix = true);


	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Find the direction closer to the given vector, given a set of directions.
		*/
		static FVector FindClosestVectorDirection(FVector ReferenceVector, TArray<FVector> AllowedDirections);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Find the direction closer to the given vector, chosing between X and Y axes.
		*/
		static FVector FindClosestAxisDirectionXY(FVector ReferenceVector);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Find the direction closer to the given vector, chosing among X, Y, Z axes.
		*/
		static FVector FindClosestAxisDirectionXYZ(FVector ReferenceVector);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Calculate the Manhattan distance on the XY plane (the sum of distances along each axis).
		*/
		static float ManhattanDistanceXY(FVector Position1, FVector Position2);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Calculate the Manhattan distance in 3D space (the sum of distances along each axis).
		*/
		static float ManhattanDistanceXYZ(FVector Position1, FVector Position2);


	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Toggle the outline rendering on or off with the given mode for an actor, skipping components with the given tag.
		@see TogglePrimitiveOutline.
		*/
		static void ToggleActorOutline(AActor* Actor, bool Enabled, EEditFlags EditFlags, FName SkipTag = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Toggle the outline rendering on or off with the given mode for a primitive.
		@note A post-process volume is needed and proper post-process material must be set.
		*/
		static void TogglePrimitiveOutline(UPrimitiveComponent* PrimitiveComponent, bool Enabled, EEditFlags EditFlags);


	//UFUNCTION(BlueprintCallable, Category = "Util")

		/**
		Save the materials of the components of an actor.
		@see SetActorMaterials, SetActorMaterial
		*/
		static void SaveActorMaterials(AActor* Actor, TMap<UPrimitiveComponent*, FMaterialList>& Materials);
	
	//UFUNCTION(BlueprintCallable, Category = "Util")

		/**
		Set the materials of the components of an actor previously saved with SaveActorMaterials().
		@see SaveActorMaterials, SetActorMaterial
		*/
		static void SetActorMaterials(AActor* Actor, const TMap<UPrimitiveComponent*, FMaterialList>& Materials);
	
	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Replace the materials of all the components of an actor with the given material.
		@see SaveActorMaterials, SetActorMaterial
		*/
		static void SetActorMaterial(AActor* Actor, UMaterialInterface* NewMaterial, FName SkipTag = TEXT(""));
	
	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Save the materials of a primitive component.
		@see SetPrimMaterials, SetPrimMaterial, SetActorMaterials, SetActorMaterial
		*/
		static void SavePrimMaterials(UPrimitiveComponent* PrimitiveComponent, TArray<UMaterialInterface*>& Materials);
	
	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Set the materials of a primitive component previously saved with SavePrimMaterials().
		@see SavePrimMaterials, SetPrimMaterial, SetActorMaterials, SetActorMaterial
		*/
		static void SetPrimMaterials(UPrimitiveComponent* PrimitiveComponent, const TArray<UMaterialInterface*>& Materials);
	
	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Replace the materials of a primitive component with the given material.
		@see SavePrimMaterials, SetPrimMaterials, SetActorMaterials, SetActorMaterial
		*/
		static void SetPrimMaterial(UPrimitiveComponent* PrimitiveComponent, UMaterialInterface* Material);

	protected:
		static void SaveActorMaterialsInternal(AActor* Actor, TMap<UPrimitiveComponent*, FMaterialList>& Materials);

};
