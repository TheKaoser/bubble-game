// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "PaperBubble.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API APaperBubble : public APaperCharacter
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Function to move the bubble
    void Move(float Value);
};
