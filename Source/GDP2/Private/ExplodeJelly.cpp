// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplodeJelly.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AExplodeJelly::AExplodeJelly()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	JellyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JellyMesh"));
	RootComponent = JellyMesh;

	FireParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireParticleSystemComponent"));
	FireParticleSystemComponent-> SetupAttachment(RootComponent);
	FireParticleSystemComponent-> bAutoActivate = false;

	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));
	RadialForce-> Radius = ExplosionRadius;
	RadialForce-> bImpulseVelChange = true;
	RadialForce-> bAutoActivate = false;
//To ignore other jelly and prevent them from exploding also
	RadialForce-> bIgnoreOwningActor = false;

	OnTakeAnyDamage.AddDynamic(this, &AExplodeJelly::OnDamageTaken);

	HitPoints = 10.0f;
	ExplosionDamage = 5.0f;
	ExplosionRadius = 200.0f;
	
}

// Called when the game starts or when spawned
void AExplodeJelly::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplodeJelly::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplodeJelly::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage > 0.f && HitPoints > 0.f)
	{
		HitPoints-= Damage;
		UE_LOG(LogTemp, Warning, TEXT("Damage taken, hit points: %f"), HitPoints); //Log the remaining hit points (shows up in the output log)

		//Start the fire particle system
		if(!FireParticleSystemComponent->IsActive())
		{
			FireParticleSystemComponent->Activate();
		}

		//Start a countdown, if not started
		if(!GetWorld()->GetTimerManager().IsTimerActive(CountdownTimerHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Setting countdown timer")); //Log if the timer is being set
			GetWorld()-> GetTimerManager().SetTimer(CountdownTimerHandle, this, &AExplodeJelly::Countdown, 1.0f, true);
		}
	}
	//If hit points reach 0, the jelly explodes
	if(HitPoints<= 0.f)
	{
		//Stop fire particle system
		if(FireParticleSystemComponent!= nullptr)
		{
			FireParticleSystemComponent->Deactivate();
		}
		
		//Cache location for spawning events
		FVector JellyLocation = GetActorLocation();

		Destroy();

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, JellyLocation, ExplosionRadius, nullptr, TArray<AActor*>(), this);

		if(RadialForce!= nullptr)
		{
			RadialForce->FireImpulse();
		}

		//Play explosion sound and particle system
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, JellyLocation);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, JellyLocation);
	}
}

void AExplodeJelly::Countdown()
{
	HitPoints--;
	UE_LOG(LogTemp, Warning, TEXT("Countdown called, hit points: %f"), HitPoints); //Log when the countdown function is called, and the remaining hit points

	//If hit points reach 0, the jelly explodes
	if(HitPoints<=0.f)
	{
		OnDamageTaken(this, 0.0f, nullptr, nullptr, nullptr);
	}
}
