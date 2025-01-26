// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "ParalaxBackground.generated.h"

UCLASS()
class MYPROJECT_API AParalaxBackground : public APaperSpriteActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AParalaxBackground();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class APaperBubble* PaperBubble;
};
