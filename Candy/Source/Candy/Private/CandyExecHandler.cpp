// Copyright DJ Song Super-Star. All Rights Reserved.

#include "CandyExecHandler.h"
#include "CandyExec.h"
#include "CandyModule.h"
#include "CandySettings.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/MessageDialog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

#if WITH_EDITOR
namespace Candy
{
	/** To get the "content" only. */
	bool ParseChatGPTResponseJson(const FString& JsonString, FString& OutContentString)
	{
		bool bRetVal = false;

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
			if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
			{
				TSharedPtr<FJsonObject> MessageObj = (*ChoicesArray)[0]->AsObject()->GetObjectField(TEXT("message"));
				OutContentString = MessageObj->GetStringField(TEXT("content"));
				bRetVal = true;
			}
			else
			{
				UE_LOG(LogCandy, Error, TEXT("No 'choices' array found in response."));
			}
		}
		else
		{
			UE_LOG(LogCandy, Error, TEXT("Failed to parse JSON response."));
		}
		return bRetVal;
	}

	bool SendChatGPTRequestInner(const FString& RequestMessage)
	{
		const UCandySettings* CandySettings = GetDefault<UCandySettings>();

		if (CandySettings->ChatGPTApiKey.Len() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
				FString(
					TEXT("Please provide ChatGPT API Key\r\n")
				)
			));
			return false;
		}

		const FString Url = TEXT("https://api.openai.com/v1/chat/completions");

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

		// Set headers
		Request->SetURL(Url);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *CandySettings->ChatGPTApiKey));

		// Create JSON payload
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("model"), TEXT("gpt-4o"));

		TArray<TSharedPtr<FJsonValue>> Messages;
		TSharedPtr<FJsonObject> Msg = MakeShareable(new FJsonObject);
		Msg->SetStringField(TEXT("role"), TEXT("user"));
		Msg->SetStringField(TEXT("content"), RequestMessage);
		Messages.Add(MakeShareable(new FJsonValueObject(Msg)));
		JsonObject->SetArrayField(TEXT("messages"), Messages);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		Request->SetContentAsString(OutputString);
		Request->OnProcessRequestComplete().BindStatic(&OnChatGPTResponse);

		Request->ProcessRequest();

		return true;

	}

	bool SendChatGPTCmdRequest(const FString& RequestMessage)
	{
		const FString WholeQuestion = FString::Printf(
			TEXT("Please reply in the form of one of commands below for the question\r\n")
			TEXT("\"%s Asset=MeshAssetName CoordX=# CoordY=# CoordZ=#\"\r\n")
			TEXT("\"%s Asset=MeshAssetName Name=ActorName CoordX=# CoordY=# CoordZ=#\" (In this command you can specify either asset or name) \r\n")
			TEXT("\"%s Asset=MeshAssetName Name=ActorName ScaleX=# ScaleY=# ScaleZ=# SizeX=# SizeY=# SizeZ=#\" (Specify either size or scale, not both) \r\n")
			TEXT("It is suggested that make multiple execution of a command if needed, separate between commands by new line.\r\n")
			TEXT("Question is : %s"),
			*FCandyExec::CMD_ADD_STATICMESH_ACTOR,
			*FCandyExec::CMD_MOVE_STATICMESH_ACTOR,
			*FCandyExec::CMD_SCALE_STATICMESH_ACTOR,
			*RequestMessage
		);

		return SendChatGPTRequestInner(WholeQuestion);
	}

	bool SendChatGPTSimpleMessage(const FString& Message)
	{
		return SendChatGPTRequestInner(Message);
	}

	void OnChatGPTResponse(FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			const FString ResponseString = Response->GetContentAsString();

			FString ResponseContentString;
			if (ParseChatGPTResponseJson(ResponseString, ResponseContentString))
			{
				UE_LOG(LogCandy, Display, TEXT("Candy Response : %s"), *ResponseContentString);

				FString ResponseContentStringProcessing = *ResponseContentString;

				//int32 StrPtr = 0;
				while (ResponseContentStringProcessing.Len() > 0)
				{
					int32 FindCmdIndex = ResponseContentStringProcessing.Find(FCandyExec::CMD_ADD_STATICMESH_ACTOR);
					if (FindCmdIndex == INDEX_NONE)
					{
						FindCmdIndex = ResponseContentStringProcessing.Find(FCandyExec::CMD_MOVE_STATICMESH_ACTOR);
					}
					if (FindCmdIndex == INDEX_NONE)
					{
						FindCmdIndex = ResponseContentStringProcessing.Find(FCandyExec::CMD_SCALE_STATICMESH_ACTOR);
					}
					/*if (FindCmdIndex == INDEX_NONE)
					{
						FindCmdIndex = ResponseContentStringProcessing.Find(...);
					}*/
					if (FindCmdIndex == INDEX_NONE)
					{
						break;
					}

					ResponseContentStringProcessing = ResponseContentStringProcessing.Mid(FindCmdIndex);
					int32 NewLineIndex;
					FString SingleCmd;
					// Asked to separate between commands by new line, assume ChatGPT follows.
					if (ResponseContentStringProcessing.FindChar(TEXT('\n'), NewLineIndex))
					{
						SingleCmd = ResponseContentStringProcessing.Left(NewLineIndex);
						ResponseContentStringProcessing.RemoveFromStart(SingleCmd);
						GEngine->DeferredCommands.Add(SingleCmd);
					}
					else
					{
						// The last command might not end with \n
						GEngine->DeferredCommands.Add(ResponseContentStringProcessing);
						break;
					}
				}

			}
			else
			{
				UE_LOG(LogCandy, Display, TEXT("Candy Response As whole: %s"), *ResponseString);
			}
		}
		else
		{
			UE_LOG(LogCandy, Error, TEXT("Candy Request failed"));
		}

	}

	/** Conditional that zero component doesn't affect the current value. */
	void ConditionalAdjustStaticMeshCompScale(UStaticMeshComponent* InTargetComp, const FVector InScale3D)
	{
		if (IsValid(InTargetComp) && IsValid(InTargetComp->GetStaticMesh()))
		{
			FVector NewCompScale3D = InTargetComp->GetComponentScale();
			
			if (FMath::Abs(InScale3D.X) > 0.0f)
			{
				NewCompScale3D.X = InScale3D.X;
			}
			if (FMath::Abs(InScale3D.Y) > 0.0f)
			{
				NewCompScale3D.Y = InScale3D.Y;
			}
			if (FMath::Abs(InScale3D.Z) > 0.0f)
			{
				NewCompScale3D.Z = InScale3D.Z;
			}

			InTargetComp->SetWorldScale3D(NewCompScale3D);
		}
	}

	/** Adjust the scale of staticmeshcomponent to make it approximately in specified size. 
	 * Zero component for maintain current scale. */
	void ConditionalAdjustStaticMeshCompScaleForSize(UStaticMeshComponent* InTargetComp, const FVector InApproxSize3D)
	{
		if (IsValid(InTargetComp) && IsValid(InTargetComp->GetStaticMesh()))
		{
			FVector NewCompScale3D = InTargetComp->GetComponentScale();
			const FBoxSphereBounds& MeshBound = InTargetComp->GetStaticMesh()->GetBounds();
			// It might be ridiculous, but can handle negative scale.
			if (FMath::Abs(InApproxSize3D.X) > 0.0f)
			{
				NewCompScale3D.X = InApproxSize3D.X / (2.0f * static_cast<float>(MeshBound.BoxExtent.X));
			}
			if (FMath::Abs(InApproxSize3D.Y) > 0.0f)
			{
				NewCompScale3D.Y = InApproxSize3D.Y / (2.0f * static_cast<float>(MeshBound.BoxExtent.Y));
			}
			if (FMath::Abs(InApproxSize3D.Z) > 0.0f)
			{
				NewCompScale3D.Z = InApproxSize3D.Z / (2.0f * static_cast<float>(MeshBound.BoxExtent.Z));
			}

			InTargetComp->SetWorldScale3D(NewCompScale3D);
		}
	}

	/** Returns whether InCheckActor is what do we looking for by either MeshAssetSearchKey or ActorNameSearchKey  */
	bool IsKeyMatchingSmActor(AStaticMeshActor* InCheckActor, const FString& MeshAssetSearchKey, const FString& ActorNameSearchKey)
	{
		if (IsValid(InCheckActor) && IsValid(InCheckActor->GetStaticMeshComponent()->GetStaticMesh()))
		{
			const bool bMeshMatch = (MeshAssetSearchKey.Len() > 0 && InCheckActor->GetStaticMeshComponent()->GetStaticMesh()->GetName().Contains(MeshAssetSearchKey));
			// In the case of name, should match exactly.
			const bool bNameMatch = (ActorNameSearchKey.Len() > 0 && InCheckActor->GetName() == ActorNameSearchKey);

			return (bMeshMatch || bNameMatch);
		}
		return false;
	}

	void HandleCmdAddStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd)
	{
		FString MeshAssetSearchKey;
		FParse::Value(Cmd, TEXT("Asset="), MeshAssetSearchKey);

		float CoordX = 0.0f, CoordY = 0.0f, CoordZ = 0.0f;
		FParse::Value(Cmd, TEXT("CoordX="), CoordX);
		FParse::Value(Cmd, TEXT("CoordY="), CoordY);
		FParse::Value(Cmd, TEXT("CoordZ="), CoordZ);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add(TEXT("/Game")); // Search only in /Game
		Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine"), UStaticMesh::StaticClass()->GetFName())); // Optional
		//Filter.PackageNames.Add(*MeshAssetSearchKey); //<- Doesn't seem like work
		TArray<FAssetData> FoundAssetDataList;
		AssetRegistry.GetAssets(Filter, FoundAssetDataList);
		
		FoundAssetDataList.RemoveAll([&MeshAssetSearchKey](FAssetData& AssetData)
		{ 
			return (false == AssetData.AssetName.ToString().Contains(MeshAssetSearchKey));
		});
		
		// Well there could be nothing then let's use anything.
		if (FoundAssetDataList.Num() == 0)
		{
			Filter.PackageNames.Empty();
			AssetRegistry.GetAssets(Filter, FoundAssetDataList);
		}

		UStaticMesh* LoadedMeshAsset = nullptr;
		if (FoundAssetDataList.Num() > 0)
		{
			const FAssetData& SelectedData = FoundAssetDataList[FMath::RandRange(0, FoundAssetDataList.Num() - 1)];
			LoadedMeshAsset = Cast<UStaticMesh>(SelectedData.GetAsset());
		}
		if (IsValid(LoadedMeshAsset))
		{
			FActorSpawnParameters SpawnParam;
			FTransform SpawnTransfom = FTransform::Identity;
			SpawnTransfom.SetLocation(FVector(CoordX, CoordY, CoordZ));
			AStaticMeshActor* NewActor = InWorld->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnTransfom, SpawnParam);
			if (IsValid(NewActor))
			{
				NewActor->GetStaticMeshComponent()->SetStaticMesh(LoadedMeshAsset);
			}
		}
	}

	void HandleCmdMoveStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd)
	{
		FString MeshAssetSearchKey;
		FParse::Value(Cmd, TEXT("Asset="), MeshAssetSearchKey);

		FString ActorNameSearchKey;
		FParse::Value(Cmd, TEXT("Name="), ActorNameSearchKey);

		float CoordX = 0.0f, CoordY = 0.0f, CoordZ = 0.0f;
		FParse::Value(Cmd, TEXT("CoordX="), CoordX);
		FParse::Value(Cmd, TEXT("CoordY="), CoordY);
		FParse::Value(Cmd, TEXT("CoordZ="), CoordZ);

		for (FActorIterator ItActor(InWorld); ItActor; ++ItActor)
		{
			AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ItActor);
			if (IsValid(AsSMA) && IsKeyMatchingSmActor(AsSMA, MeshAssetSearchKey, ActorNameSearchKey))
			{
				AsSMA->SetActorLocation(FVector(CoordX, CoordY, CoordZ));
			}
		}
	}

	void HandleCmdScaleStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd)
	{
		FString MeshAssetSearchKey;
		FParse::Value(Cmd, TEXT("Asset="), MeshAssetSearchKey);

		FString ActorNameSearchKey;
		FParse::Value(Cmd, TEXT("Name="), ActorNameSearchKey);

		float ScaleX = 0.0f, ScaleY = 0.0f, ScaleZ = 0.0f;
		FParse::Value(Cmd, TEXT("ScaleX="), ScaleX);
		FParse::Value(Cmd, TEXT("ScaleY="), ScaleY);
		FParse::Value(Cmd, TEXT("ScaleZ="), ScaleZ);

		float SizeX = 0.0f, SizeY = 0.0f, SizeZ = 0.0f;
		FParse::Value(Cmd, TEXT("SizeX="), SizeX);
		FParse::Value(Cmd, TEXT("SizeY="), SizeY);
		FParse::Value(Cmd, TEXT("SizeZ="), SizeZ);

		for (FActorIterator ItActor(InWorld); ItActor; ++ItActor)
		{
			AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ItActor);
			if (IsValid(AsSMA) && IsKeyMatchingSmActor(AsSMA, MeshAssetSearchKey, ActorNameSearchKey))
			{
				// If both scale and size are specified, size has higher priority by applied later.
				ConditionalAdjustStaticMeshCompScale(AsSMA->GetStaticMeshComponent(), FVector(ScaleX, ScaleY, ScaleZ));
				ConditionalAdjustStaticMeshCompScaleForSize(AsSMA->GetStaticMeshComponent(), FVector(SizeX, SizeY, SizeZ));
			}
		}
	}
}
#endif // WITH_EDITOR
