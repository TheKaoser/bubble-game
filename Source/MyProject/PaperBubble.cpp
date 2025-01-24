// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperBubble.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "GameFramework/CharacterMovementComponent.h"

APaperBubble::APaperBubble()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
}

void APaperBubble::BeginPlay()
{
    Super::BeginPlay();
}

void APaperBubble::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Example movement logic: move right continuously
    // Move(1.0f);
}

void APaperBubble::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &APaperBubble::MoveUp);
        EnhancedInputComponent->BindAction(MoveDownAction, ETriggerEvent::Triggered, this, &APaperBubble::MoveDown);
        EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APaperBubble::MoveRight);
        EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &APaperBubble::MoveLeft);
    }
}

void APaperBubble::MoveUp(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, MovementValue * 10.0f), true);
    }
}

void APaperBubble::MoveDown(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, -MovementValue * 10.0f), true);
    }
}

void APaperBubble::MoveRight(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        GetCharacterMovement()->AddImpulse(FVector(MovementValue * 10.0f, 0.0f, 0.0f), true);
    }    
}

void APaperBubble::MoveLeft(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        GetCharacterMovement()->AddImpulse(FVector(-MovementValue * 10.0f, 0.0f, 0.0f), true);
    }
}