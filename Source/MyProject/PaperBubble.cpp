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
#include "HitActor.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

APaperBubble::APaperBubble()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APaperBubble::OnOverlapBegin);
}

void APaperBubble::BeginPlay()
{
    Super::BeginPlay();

    // set location to (X=-342.500000,Y=142.500000,Z=-1081.786864)
    SetActorLocation(FVector(-342.5f, 142.5f, 0));
}

void APaperBubble::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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

void APaperBubble::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->IsA(AHitActor::StaticClass()))
        {
            UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
        }
    }
}