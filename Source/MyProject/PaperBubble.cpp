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
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MediaPlateComponent.h"
#include "MediaPlayer.h"
#include "EngineUtils.h"

APaperBubble::APaperBubble()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APaperBubble::OnOverlapBegin);
    GetCapsuleComponent()->SetUseCCD(true);

    CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
    CableComponent->SetupAttachment(RootComponent);
    CableComponent->SetHiddenInGame(true); 
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> CableMaterial(TEXT("Material'/Game/Materials/M_CableMaterial'"));
    if (CableMaterial.Succeeded())
    {
        CableComponent->SetMaterial(0, CableMaterial.Object);
        UE_LOG(LogTemp, Warning, TEXT("Cable material successfully applied."));
    }
    CableComponent->CableWidth = 5.0f;
}

void APaperBubble::BeginPlay()
{
    Super::BeginPlay();

    CurrentBubbleType = BubbleType::SoapBubble;
    
    for (TObjectIterator<APaperFlipbookActor> It; It; ++It)
    {
        if (It->GetWorld() && (It->GetWorld()->WorldType == EWorldType::PIE || It->GetWorld()->WorldType == EWorldType::Game))
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
        if (It->GetWorld() && (It->GetWorld()->WorldType == EWorldType::PIE || It->GetWorld()->WorldType == EWorldType::Game))
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
                GetSprite()->SetPlaybackPositionInFrames(0, false);
                GetSprite()->SetFlipbook(AirIdle);
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
                GetCharacterMovement()->GravityScale = .15f;
                GetSprite()->SetPlaybackPositionInFrames(0, false);
                GetSprite()->SetFlipbook(GumIdle);
            }
        }
    }

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = .0f;
    CurrentBubbleType = BubbleType::TransitionBubble;
    SetActorHiddenInGame(true);

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::StartGame, 12.0f, false);
}

void APaperBubble::StartGame()
{
    CurrentBubbleType = BubbleType::SoapBubble;
    GetCharacterMovement()->GravityScale = .1f;
    SetActorHiddenInGame(false);

    for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
    {
        if (It->GetWorld() && (It->GetWorld()->WorldType == EWorldType::PIE || It->GetWorld()->WorldType == EWorldType::Game))
        {
            if (It->ComponentHasTag("StartCinematic"))
            {
                It->SetHiddenInGame(true);
                break;
            }
        }
    }
    // WinGame();
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

    if (CameraLocation.Z < -2800.0f)
    {
        CameraLocation.Z = -2800.0f;
        CameraComponent->SetWorldLocation(CameraLocation);
    }

    CurrentCoolDown -= DeltaTime;

    if (FMath::IsNearlyEqual(GetActorLocation().Z, LastFrameZ, 0.01f) and GetCharacterMovement()->Velocity.Size() < 0.01f and CurrentBubbleType == BubbleType::GumBubble and not killed)
    {
        KillBubble();
        killed = true;
    }
    LastFrameZ = GetActorLocation().Z;

    if (GetActorLocation().Z < -2800.0f and CurrentBubbleType == BubbleType::GumBubble)
    {
        WinGame();
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
        {
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, MovementValue * 10.0f), true);
        }
    }
}

void APaperBubble::MoveDown(const FInputActionValue& Value)
{
    float MovementValue = Value.Get<float>();
    if (MovementValue != 0.0f)
    {
        if (CurrentBubbleType == BubbleType::SoapBubble)
        {
            GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, -MovementValue * 10.0f), true);
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
        else if (CurrentBubbleType == BubbleType::GumBubble and CurrentCoolDown <= 0.0f)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(1000.0f, 0.0f, 0.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(10000.0f, 0.0f, 0.0f), false);
                    CurrentCoolDown = CoolDown;

                    CableComponent->SetHiddenInGame(false);
                    CableComponent->SetAttachEndToComponent(HitResult.GetComponent(), HitResult.BoneName);
                    FVector LocalImpactPoint = HitResult.GetComponent()->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);
                    CableComponent->EndLocation = LocalImpactPoint;

                    FTimerHandle TimerHandle;
                    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::HideCable, 1.0f, false);
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
        else if (CurrentBubbleType == BubbleType::GumBubble and CurrentCoolDown <= 0.0f)
        {
            FHitResult HitResult;
            FVector Start = GetActorLocation();
            FVector End = Start + FVector(-1000.0f, 0.0f, 0.0f);
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 1.0f);

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams))
            {
                if (HitResult.GetActor()->Tags.Contains("AttachmentPoint"))
                {
                    GetCharacterMovement()->StopMovementImmediately();
                    GetCharacterMovement()->AddImpulse(FVector(-10000.0f, 0.0f, 0.0f), false);
                    CurrentCoolDown = CoolDown;

                    CableComponent->SetHiddenInGame(false);
                    CableComponent->SetAttachEndToComponent(HitResult.GetComponent(), HitResult.BoneName);
                    FVector LocalImpactPoint = HitResult.GetComponent()->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);
                    CableComponent->EndLocation = LocalImpactPoint;

                    FTimerHandle TimerHandle;
                    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::HideCable, 0.3f, false);
                }
            }
        }
    }
}

void APaperBubble::HideCable()
{
    CableComponent->SetHiddenInGame(true);
    CableComponent->SetAttachEndTo(nullptr, NAME_None);
    CableComponent->CableLength = 0.0f;
}

void APaperBubble::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->IsA(AHitActor::StaticClass()))
        {
            if (OtherActor->Tags.Contains("EndLevel"))
            {
                KillBubble();
            }
            else if (OtherActor->Tags.Contains("NextLevel"))
            {
                ChangeBehavior();
            }
        }
    }
}

void APaperBubble::KillBubble()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = .0f;
    
    GetSprite()->SetFlipbook(CurrentBubbleType == BubbleType::SoapBubble ? AirDeath : GumDeath);
    GetSprite()->SetPlaybackPositionInFrames(0, false);
    GetSprite()->SetLooping(false);

    OnBubbleDeath();

    CurrentBubbleType = BubbleType::TransitionBubble;

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::ResetLevel, 3.0f, false);
}

void APaperBubble::ResetLevel()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void APaperBubble::ChangeBehavior()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = .0f;
    CurrentBubbleType = BubbleType::TransitionBubble;
    SetActorHiddenInGame(true);

    FTimerHandle TimerHandle;

    PaperFlipbookActor->SetActorHiddenInGame(false);
    PaperFlipbookActor->GetRenderComponent()->SetPlaybackPositionInFrames(0, false);
    PaperFlipbookActor->GetRenderComponent()->SetLooping(false);

    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::ChangeLevel, 1.8f, false);
}

void APaperBubble::ChangeLevel()
{
    OnLevelChange();
    for (TObjectIterator<AActor> It; It; ++It)
    {
        if (It->GetWorld() && (It->GetWorld()->WorldType == EWorldType::PIE || It->GetWorld()->WorldType == EWorldType::Game))
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
            }
        }
    }

    CurrentBubbleType = BubbleType::GumBubble;
    SetActorHiddenInGame(false);
    GetSprite()->SetPlaybackPositionInFrames(0, false);
    GetSprite()->SetFlipbook(GumIdle);
    PaperFlipbookActor->GetRenderComponent()->SetLooping(true);
    GetCharacterMovement()->GravityScale = .15f;
}

void APaperBubble::WinGame()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->GravityScale = .0f;
    CurrentBubbleType = BubbleType::TransitionBubble;
    SetActorHiddenInGame(true);

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->ActorHasTag("EndAnim") && (Actor->GetWorld()->WorldType == EWorldType::PIE || Actor->GetWorld()->WorldType == EWorldType::Game))
        {
            Actor->SetActorLocation(FVector(20.0f, -60.0f, -2800.0f));
            if (UMediaPlateComponent* MediaPlateComponent = Actor->FindComponentByClass<UMediaPlateComponent>())
            {
                if (MediaPlateComponent->GetMediaPlayer())
                {
                    MediaPlateComponent->Open();
                    MediaPlateComponent->Play();
                }
            }
            break;
        }
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &APaperBubble::ResetLevel, 12.0f, false);
}