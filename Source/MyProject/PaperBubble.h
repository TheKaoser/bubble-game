// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "InputActionValue.h"
#include "PaperBubble.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API APaperBubble : public APaperCharacter
{
	GENERATED_BODY()

private:
    APaperBubble();

    UPROPERTY(EditAnywhere, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* MoveUpAction;
    UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* MoveDownAction;
    UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* MoveRightAction;
    UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* MoveLeftAction;

    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* CameraComponent;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    void MoveUp(const FInputActionValue& Value);
    void MoveDown(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void MoveLeft(const FInputActionValue& Value);
};
