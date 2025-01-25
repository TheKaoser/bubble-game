#include "HitActor.h"
#include "Components/SphereComponent.h"

// Sets default values
AHitActor::AHitActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the attachment point component
    AttachmentPoint = CreateDefaultSubobject<USphereComponent>(TEXT("AttachmentPoint"));
    AttachmentPoint->SetupAttachment(RootComponent);
    AttachmentPoint->SetSphereRadius(20.0f);
    AttachmentPoint->SetCollisionProfileName(TEXT("OverlapAll"));
}

// Called when the game starts or when spawned
void AHitActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHitActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
