// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBombActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AFPSBombActor::AFPSBombActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	RadiusShape = 500.0f;
	ExplodeDelay = 2.0f;
}

// Called when the game starts or when spawned
void AFPSBombActor::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Explode_TimerHandle;
	GetWorldTimerManager().SetTimer(Explode_TimerHandle, this, &AFPSBombActor::Explode, ExplodeDelay);

	MaterialInst =  MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	if(MaterialInst)
	{
		CurrentColor = MaterialInst->K2_GetVectorParameterValue("Color");
	}

	TargetColor = FLinearColor::MakeRandomColor();
	
}

void AFPSBombActor::Explode()
{
	//explode
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionTemplate, GetActorLocation());

	FCollisionObjectQueryParams QueryParameters; 
	QueryParameters.AddObjectTypesToQuery(ECC_WorldDynamic);
	QueryParameters.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionShape CollShape;
	CollShape.SetSphere(RadiusShape);

	TArray<FOverlapResult> OutOverlaps;
	GetWorld()->OverlapMultiByObjectType(OutOverlaps, GetActorLocation(), FQuat::Identity, QueryParameters, CollShape);

	for (FOverlapResult Result : OutOverlaps)
	{
		UPrimitiveComponent* Overlap = Result.GetComponent();
		if(Overlap && Overlap->IsSimulatingPhysics())
		{
			UMaterialInstanceDynamic* MatInst = Overlap->CreateAndSetMaterialInstanceDynamic(0);
			if(MatInst)
			{
				MatInst->SetVectorParameterValue("Color", TargetColor);
			}
		}
	}

	Destroy();
}

// Called every frame
void AFPSBombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MaterialInst)
	{
		float Progress = (GetWorld()->TimeSeconds - CreationTime) / ExplodeDelay;

		FLinearColor NewColor = FLinearColor::LerpUsingHSV(CurrentColor, TargetColor, Progress);

		MaterialInst->SetVectorParameterValue("Color", NewColor);
	}

}

