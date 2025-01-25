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

APaperBubble::APaperBubble()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APaperBubble::OnOverlapBegin);

    // Create and attach the physics constraint component
    PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
    PhysicsConstraint->SetupAttachment(RootComponent);

    // Create and attach the cable component
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

    // set location to (X=-342.500000,Y=142.500000,Z=-1081.786864)
    SetActorLocation(FVector(-342.5f, 142.5f, 0));
    CurrentBubbleType = BubbleType::SoapBubble;
}

void APaperBubble::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (TObjectIterator<AHitActor> It; It; ++It)
    {
        if (It->GetWorld() && It->GetWorld()->WorldType == EWorldType::PIE and CurrentBubbleType == BubbleType::GumBubble)
        {
            AActor* OtherActor = *It;
            if (OtherActor->Tags.Contains("AttachmentPoint"))
                UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation()));
            if (OtherActor->Tags.Contains("AttachmentPoint") && FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation()) < 1000)
            {
                // if not attached
                UE_LOG(LogTemp, Warning, TEXT("Attach to object!"));
                AttachToObject(OtherActor);
                
            }
        }
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
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, MovementValue * 10.0f), true);
        // else change flipbook
    }
}

void APaperBubble::MoveDown(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, -MovementValue * 10.0f), true);
    }
}

void APaperBubble::MoveRight(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(MovementValue * 10.0f, 0.0f, 0.0f), true);
    }    
}

void APaperBubble::MoveLeft(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
            GetCharacterMovement()->AddImpulse(FVector(-MovementValue * 10.0f, 0.0f, 0.0f), true);
    }
}

void APaperBubble::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->IsA(AHitActor::StaticClass()))
        {
            // if tag == "EndLevel" then open the same level
            if (OtherActor->Tags.Contains("EndLevel"))
                UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
            else if (OtherActor->Tags.Contains("NextLevel"))
            {
                ChangeBehavior();
                // Change map
            }
        }
    }
}

void APaperBubble::AttachToObject(AActor* OtherActor)
{
    // Attach the bubble to the other actor using a physics constraint
    PhysicsConstraint->SetConstrainedComponents(GetCapsuleComponent(), NAME_None, Cast<UPrimitiveComponent>(OtherActor->GetRootComponent()), NAME_None);

    // Set the linear limits to constrain the movement
    PhysicsConstraint->SetLinearXLimit(LCM_Limited, 1000.0f); // Set the max length in the X direction
    PhysicsConstraint->SetLinearYLimit(LCM_Limited, 1000.0f); // Set the max length in the Y direction
    PhysicsConstraint->SetLinearZLimit(LCM_Limited, 1000.0f); // Set the max length in the Z direction
    
    PhysicsConstraint->SetLinearPositionDrive(true, true, true);

    PhysicsConstraint->SetAngularSwing1Limit(ACM_Limited, 1000.0f); // Swing1 limit (e.g., pitch)
    PhysicsConstraint->SetAngularSwing2Limit(ACM_Limited, 1000.0f); // Swing2 limit (e.g., yaw)
    PhysicsConstraint->SetAngularTwistLimit(ACM_Limited, 1000.0f); 

    PhysicsConstraint->SetAngularOrientationDrive(true, true);

    GetCapsuleComponent()->SetSimulatePhysics(true);

    CableComponent->SetVisibility(true);
    CableComponent->SetAttachEndToComponent(Cast<USceneComponent>(OtherActor->GetRootComponent()));
    CableComponent->EndLocation = FVector::ZeroVector;
    
    UE_LOG(LogTemp, Warning, TEXT("Attached to object!"));
}

void APaperBubble::ChangeBehavior()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = 1.0f;
    CurrentBubbleType = BubbleType::GumBubble;
    // simulate physics
    GetCapsuleComponent()->SetSimulatePhysics(true);
}