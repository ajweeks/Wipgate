#include "Step.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"

void UStep::SetBlockColor(UStaticMeshComponent * block, FColor color)
{
	UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(block->GetStaticMesh()->GetMaterial(0), block);
	mat->SetVectorParameterValue(FName(TEXT("Color")), color);
	block->SetMaterial(0, mat);
}
