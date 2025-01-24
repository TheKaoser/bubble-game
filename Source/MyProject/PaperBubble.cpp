// Fill out your copyright notice in the Description page of Project Settings.


#include "PaperBubble.h"

void APaperBubble::BeginPlay()
{
    Super::BeginPlay();
}

void APaperBubble::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Example movement logic: move right continuously
    Move(1.0f);
}

void APaperBubble::Move(float Value)
{
    // Add movement input to the character
    AddMovementInput(FVector(Value, 0.0f, 0.0f));
}