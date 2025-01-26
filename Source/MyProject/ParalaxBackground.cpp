#include "ParalaxBackground.h"
#include "PaperBubble.h"

// Sets default values
AParalaxBackground::AParalaxBackground()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// add to ECollisionChannel::ECC_Visibility
	
}

// Called when the game starts or when spawned
void AParalaxBackground::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AParalaxBackground::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PaperBubble)
    {
        // Get the current position of the bubble
        FVector BubbleLocation = PaperBubble->GetActorLocation();

        // Calculate the new background position based on the bubble's position and parallax speed
        FVector NewBackgroundLocation = GetActorLocation();
        NewBackgroundLocation.Z = BubbleLocation.Z * 10.f;

        // Set the new background position
        SetActorLocation(NewBackgroundLocation);
	}
}
