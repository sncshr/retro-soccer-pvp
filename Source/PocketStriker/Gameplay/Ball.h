// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ball.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;

/**
 * Ball actor with physics simulation and possession system
 * Handles kick, tackle, and pass interactions
 */
UCLASS()
class POCKETSTRIKER_API ABall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABall();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** Apply kick impulse to the ball */
	UFUNCTION(BlueprintCallable, Category = "Ball")
	void Kick(const FVector& Direction, float Force);

	/** Apply pass impulse with aim assist */
	UFUNCTION(BlueprintCallable, Category = "Ball")
	void Pass(AActor* TargetActor, float Force);

	/** Attempt to gain possession of the ball */
	UFUNCTION(BlueprintCallable, Category = "Ball")
	bool TryGainPossession(AActor* NewOwner);

	/** Release possession of the ball */
	UFUNCTION(BlueprintCallable, Category = "Ball")
	void ReleasePossession();

	/** Check if ball is currently possessed */
	UFUNCTION(BlueprintPure, Category = "Ball")
	bool IsPossessed() const { return PossessingActor != nullptr; }

	/** Get the actor currently possessing the ball */
	UFUNCTION(BlueprintPure, Category = "Ball")
	AActor* GetPossessingActor() const { return PossessingActor; }

	/** Get ball velocity */
	UFUNCTION(BlueprintPure, Category = "Ball")
	FVector GetBallVelocity() const;

	/** Predict ball trajectory for interception */
	UFUNCTION(BlueprintCallable, Category = "Ball")
	FVector PredictPositionAtTime(float Time) const;

protected:
	/** Called when ball overlaps with another actor */
	UFUNCTION()
	void OnBallOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Update ball possession state */
	void UpdatePossession(float DeltaTime);

	/** Update visual feedback for possession */
	void UpdateVisualFeedback();

protected:
	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComponent;

	/** Static mesh for ball visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Particle effect for possession highlight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* PossessionEffect;

	/** Actor currently possessing the ball */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
	AActor* PossessingActor;

	/** Physics properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BallMass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BallFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BallRestitution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BallLinearDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BallAngularDamping;

	/** Possession properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Possession")
	float PossessionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Possession")
	float PossessionMinVelocity;

	/** Kick properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float MaxKickForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float PassAimAssistRadius;

	/** Visual feedback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor PossessionHighlightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bShowPossessionEffect;

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
