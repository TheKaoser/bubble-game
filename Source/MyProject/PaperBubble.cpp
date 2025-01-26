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
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CableComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbookActor.h"

APaperBubble::APaperBubble()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 500.0f;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APaperBubble::OnOverlapBegin);

    CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
    CableComponent->SetupAttachment(RootComponent);
    CableComponent->SetVisibility(false); 
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> CableMaterial(TEXT("Material'/Game/Materials/M_CableMaterial.M_CableMaterial'"));
    if (CableMaterial.Succeeded())
    {
        CableComponent->SetMaterial(0, CableMaterial.Object);
    }
    CableComponent->CableWidth = 5.0f;
}

void APaperBubble::BeginPlay()
{
    Super::BeginPlay();

    // CurrentBubbleType = BubbleType::SoapBubble;
    CurrentBubbleType = BubbleType::GumBubble;
    
    for (TObjectIterator<APaperFlipbookActor> It; It; ++It)
    {
        if (It->GetWorld() && It->GetWorld()->WorldType == EWorldType::PIE)
        {
            APaperFlipbookActor* OtherActor = *It;
            if (OtherActor->Tags.Contains("TransitionFlipboard"))
            {
                PaperFlipbookActor = OtherActor;
                PaperFlipbookActor->SetActorHiddenInGame(true);
                break;
            }
        }
    }

    for (TObjectIterator<AActor> It; It; ++It)
    {
        if (It->GetWorld() && It->GetWorld()->WorldType == EWorldType::PIE)
        {
            if (CurrentBubbleType == BubbleType::SoapBubble)
            {
                AActor* OtherActor = *It;
                if (OtherActor->Tags.Contains("Gum"))
                {
                    OtherActor->SetActorHiddenInGame(true);
                    OtherActor->SetActorEnableCollision(false);
                }
                SetActorLocation(FVector(.0f, .10f, -2800.0f));
            }
            else if (CurrentBubbleType == BubbleType::GumBubble)
            {
                AActor* OtherActor = *It;
                if (OtherActor->Tags.Contains("Air"))
                {
                    OtherActor->SetActorHiddenInGame(true);
                    OtherActor->SetActorEnableCollision(false);
                }
                SetActorLocation(FVector(.0f, .10f, 2000.0f));
                GetCharacterMovement()->GravityScale = .5f;
            }
        }
    }
}

void APaperBubble::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector CameraLocation = CameraComponent->GetComponentLocation();
    CameraLocation.X = .0f;
    CameraComponent->SetWorldLocation(CameraLocation);

    FVector ActorLocation = GetActorLocation();
    ActorLocation.Y = .10f;
    SetActorLocation(ActorLocation);

    // set z limit for camera location
    if (CameraLocation.Z < -3000.0f)
    {
        CameraLocation.Z = -3000.0f;
        CameraComponent->SetWorldLocation(CameraLocation);
    }
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
        UE_LOG(LogTemp, Warning, TEXT("MoveUp 0: %d"), CurrentBubbleType);
        if (CurrentBubbleType == BubbleType::SoapBubble)
        {
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, MovementValue * 10.0f), true);
        }
        else if (CurrentBubbleType == BubbleType::GumBubble)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(0.0f, 0.0f, 10000.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 1.0f);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.GetActor()->GetName());
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, 1000.0f), false);
                }
            }
        }
    }
}

void APaperBubble::MoveDown(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, -MovementValue * 10.0f), true);
        else if (CurrentBubbleType == BubbleType::GumBubble)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(0.0f, 0.0f, -10000.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 1.0f);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.GetActor()->GetName());
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, -1000.0f), false);
                }
            }
        }
    }
}

void APaperBubble::MoveRight(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(MovementValue * 10.0f, 0.0f, 0.0f), true);
        else if (CurrentBubbleType == BubbleType::GumBubble)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(10000.0f, 0.0f, 0.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 1.0f);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.GetActor()->GetName());
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(1000.0f, 0.0f, 0.0f), false);
                }
            }
        }
    }    
}

void APaperBubble::MoveLeft(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(-MovementValue * 10.0f, 0.0f, 0.0f), true);
        else if (CurrentBubbleType == BubbleType::GumBubble)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(-10000.0f, 0.0f, 0.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 1.0f);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.GetActor()->GetName());
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(-1000.0f, 0.0f, 0.0f), false);
                }
            }
        }
    }
}

void APaperBubble::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->IsA(AHitActor::StaticClass()))
        {
            if (OtherActor->Tags.Contains("EndLevel"))
            {
                GetCharacterMovement()->StopMovementImmediately();
                GetCharacterMovement()->GravityScale = .0f;
                
                GetSprite()->SetPlaybackPositionInFrames(0, false);

                GetSprite()->SetFlipbook(AirDeath);
                GetSprite()->SetLooping(false);

                // call OnBubbleDeath in blueprints
                OnBubbleDeath();

                CurrentBubbleType = BubbleType::TransitionBubble;

                // open level in 3 seconds
                FTimerHandle TimerHandle;
                GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::ResetLevel, 3.0f, false);
            }
            else if (OtherActor->Tags.Contains("NextLevel"))
            {
                ChangeBehavior();
            }
        }
    }
}

void APaperBubble::ResetLevel()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void APaperBubble::ChangeBehavior()
{
    UE_LOG(LogTemp, Warning, TEXT("ChangeBehavior: %d"), CurrentBubbleType);
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = .0f;
    CurrentBubbleType = BubbleType::TransitionBubble;

    SetActorHiddenInGame(true);

    FTimerHandle TimerHandle;

    PaperFlipbookActor->SetActorHiddenInGame(false);
    PaperFlipbookActor->GetRenderComponent()->SetPlaybackPositionInFrames(0, false);
    PaperFlipbookActor->GetRenderComponent()->SetLooping(false);

    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::ChangeLevel, 1.0f, false);
}

void APaperBubble::ChangeLevel()
{
    OnLevelChange();
    for (TObjectIterator<AActor> It; It; ++It)
    {
        if (It->GetWorld() && It->GetWorld()->WorldType == EWorldType::PIE)
        {
            AActor* OtherActor = *It;
            if (OtherActor->Tags.Contains("Air"))
            {
                OtherActor->SetActorHiddenInGame(true);
                OtherActor->SetActorEnableCollision(false);
            }
            else if (OtherActor->Tags.Contains("Gum"))
            {
                OtherActor->SetActorHiddenInGame(false);
                OtherActor->SetActorEnableCollision(true);
                CurrentBubbleType = BubbleType::TransitionBubble;
            }
        }
    }

    SetActorHiddenInGame(false);

    GetCharacterMovement()->GravityScale = .5f;
    GetCapsuleComponent()->SetSimulatePhysics(true);
}