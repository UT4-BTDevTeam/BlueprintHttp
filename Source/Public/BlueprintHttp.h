#pragma once

#include "Engine.h"
#include "Http.h"

#include "BlueprintHttp.generated.h"

// Using UObject has been causing issues because the object gets GC'd prematurely (typically at BeginPlay + 0.5s)
// I don't know how to prevent object from being GC'd since we only have static methods (and RootSet causes other issues)
// So now our request wrapper is a AActor spawned in the World and it doesn't get GC'd, woooo
UCLASS()
class ABlueprintHttpRequest : public AActor
{
	GENERATED_UCLASS_BODY()

	TSharedPtr<IHttpRequest> Request;
	bool Done = false;

	bool Success = false;
	int32 Code = 0;
	FString Body;
	TArray<FString> Keys;
	TArray<FString> Values;

	void AddUserHeaders(const TArray<FString>& Keys, const TArray<FString>& Values);

	void ProcessGet(const FString& Url);
	void ProcessPost(const FString& Url, const FString& JsonBody);

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void Cancel();
};

UCLASS()
class UBlueprintHttp : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Meta = (CallableWithoutWorldContext, Keywords = "http plugin detect"), Category = "BlueprintHttp")
	static bool HttpPluginDetection() { return true; };

	UFUNCTION(BlueprintCallable, meta = (
		Keywords = "http request get",
		Latent,
		AdvancedDisplay = "5",
		AutoCreateRefTerm = "HeaderKeys,HeaderValues",
		WorldContext = "WorldContextObject",
		DefaultToSelf = "WorldContextObject",
		LatentInfo = "LatentInfo"
	), Category = "BlueprintHttp")
	static void HttpGet(UObject* WorldContextObject,
		/* in */ const FString& Url,
		/* out */ bool& Success,
		/* out */ TArray<FString>& Keys,
		/* out */ TArray<FString>& Values,
		// Advanced
		/* in */ const TArray<FString>& HeaderKeys,
		/* in */ const TArray<FString>& HeaderValues,
		/* out */ int32& Code,
		/* out */ FString& Body,
		FLatentActionInfo LatentInfo
	);

	UFUNCTION(BlueprintCallable, meta = (
		Keywords = "http request post",
		Latent,
		AdvancedDisplay = "6",
		AutoCreateRefTerm = "HeaderKeys,HeaderValues",
		WorldContext = "WorldContextObject",
		DefaultToSelf = "WorldContextObject",
		LatentInfo = "LatentInfo"
	), Category = "BlueprintHttp")
	static void HttpPost(UObject* WorldContextObject,
		/* in */ const FString& Url,
		/* in */ const FString& JsonBody,
		/* out */ bool& Success,
		/* out */ TArray<FString>& Keys,
		/* out */ TArray<FString>& Values,
		// Advanced
		/* in */ const TArray<FString>& HeaderKeys,
		/* in */ const TArray<FString>& HeaderValues,
		/* out */ int32& Code,
		/* out */ FString& Body,
		FLatentActionInfo LatentInfo
	);

	static TSharedPtr<FJsonObject> FlattenJson(const TSharedPtr<FJsonObject>& InObj);
	static void FlattenJson_Internal(const TSharedPtr<FJsonValue>& InVal, TSharedPtr<FJsonObject>& OutObj, const FString& Path);

	UFUNCTION(BlueprintPure, meta = (CallableWithoutWorldContext, Keywords = "http post make json"), Category="BlueprintHttp")
	static void MakeSimpleJson(const TArray<FString>& Keys, const TArray<FString>& Values, FString& Json);

	UFUNCTION(BlueprintPure, meta = (CallableWithoutWorldContext, Keywords = "http post make json object"), Category = "BlueprintHttp")
	static void MakeJsonObject(const TArray<FString>& Keys, const TArray<FString>& Values, const TArray<FString>& Types, FString& Json);

	UFUNCTION(BlueprintPure, meta = (CallableWithoutWorldContext, Keywords = "http post make json array"), Category = "BlueprintHttp")
	static void MakeJsonArray(const TArray<FString>& Values, const TArray<FString>& Types, FString& Json);

	static FString FormatJsonValue(const FString& UserValue, const FString& Type);
};
