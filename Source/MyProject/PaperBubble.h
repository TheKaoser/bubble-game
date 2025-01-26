#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "InputActionValue.h"
#include "PaperBubble.generated.h"

UENUM(BlueprintType)
enum class BubbleType : uint8
{
    SoapBubble,
    GumBubble,
    TransitionBubble
};

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* CameraComponent;

    UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    void MoveUp(const FInputActionValue& Value);
    void MoveDown(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void MoveLeft(const FInputActionValue& Value);

    UFUNCTION()
    void ChangeBehavior();
    void ChangeLevel();
    void ResetLevel();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bubble")
    BubbleType CurrentBubbleType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCableComponent* CableComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class APaperFlipbookActor* PaperFlipbookActor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UPaperFlipbook* AirIdle;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UPaperFlipbook* AirDeath;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UPaperFlipbook* GumIdle;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UPaperFlipbook* GumDeath;

    UFUNCTION(BlueprintImplementableEvent)
    void OnBubbleDeath();

    void KillBubble();
    bool killed = false;

    UFUNCTION(BlueprintImplementableEvent)
    void OnLevelChange();

    float CoolDown = 1.0f;
    float CurrentCoolDown = 0.f;

    float LastFrameZ = .0f;
};
