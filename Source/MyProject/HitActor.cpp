#include "HitActor.h"
#include "Components/SphereComponent.h"

// Sets default values
AHitActor::AHitActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// add to ECollisionChannel::ECC_Visibility
	
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
