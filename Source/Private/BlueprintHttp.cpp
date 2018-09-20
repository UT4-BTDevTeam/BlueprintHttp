#include "BlueprintHttp.h"

#define DEBUGLOG Verbose
//#define DEBUGLOG Log

//================================================
// Latent action
//================================================

class FBlueprintHttpAction : public FPendingLatentAction
{
private:
	TSharedPtr<ABlueprintHttpRequest> Req;

	// Output pins
	bool* pSuccess;
	int32* pCode;
	FString* pBody;
	TArray<FString>* pKeys;
	TArray<FString>* pValues;

public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FBlueprintHttpAction(TSharedRef<ABlueprintHttpRequest> Req, bool* pSuccess, int32* pCode, FString* pBody, TArray<FString>* pKeys, TArray<FString>* pValues, const FLatentActionInfo& LatentInfo)
		: Req(Req)
		, pSuccess(pSuccess)
		, pCode(pCode)
		, pBody(pBody)
		, pKeys(pKeys)
		, pValues(pValues)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (Req.IsValid())
		{
			if (Req->Done)
			{
				UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] Setting pins values"));
				(*pSuccess) = Req->Success;
				(*pCode) = Req->Code;
				(*pBody) = Req->Body;
				(*pKeys) = Req->Keys;
				(*pValues) = Req->Values;
				UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] Triggering link"));
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);

				// Request is not needed anymore, clean it up
				Req->Destroy();
				Req.Reset();
			}
		}
		else
		{
			// Request wrapper got cleaned up (levelchange/exit) before request finished
			// Note that the underlying request is not level-bound and will finish anyways (unless exit?)
			UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] Update: Req null"));
			Response.DoneIf(true);
		}
	}

	// Object owning this latent node got destroyed - useless to wait for result
	// Question: should we cancel the underlying request or let it finish ?
	virtual void NotifyObjectDestroyed() override
	{
		UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] NotifyObjectDestroyed"));
		if ( Req.IsValid() )
		{
			/* We may not want an abrupt mapchange to cancel a POST request ongoing, so let the underlying request finish
			Req->Cancel();
			*/
			Req->Destroy();
			Req.Reset();
		}
	}

};

//================================================
// Static API
//================================================

UBlueprintHttp::UBlueprintHttp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBlueprintHttp::HttpGet(UObject* WorldContextObject,
	/* in */ const FString& Url,
	/* out */ bool& Success,
	/* out */ TArray<FString>& Keys,
	/* out */ TArray<FString>& Values,
	// Advanced
	/* in */ const TArray<FString>& HeaderKeys,
	/* in */ const TArray<FString>& HeaderValues,
	/* out */ int32& Code,
	/* out */ FString& Body,
	FLatentActionInfo LatentInfo)
{
	UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] HttpGet %s"), *Url);
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] World OK"));
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FBlueprintHttpAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
		{
			UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] LatentAction not found => new"));

			// The real log line for server admins
			UE_LOG(LogBlueprintUserMessages, Log, TEXT("[BlueprintHttp] Http GET %s"), *Url);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.bNoFail = true;
			TSharedRef<ABlueprintHttpRequest> Req(World->SpawnActor<ABlueprintHttpRequest>(ABlueprintHttpRequest::StaticClass(), SpawnParams));

			FBlueprintHttpAction* HttpAction = new FBlueprintHttpAction(Req, &Success, &Code, &Body, &Keys, &Values, LatentInfo);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, HttpAction);
			Req->AddUserHeaders(HeaderKeys, HeaderValues);
			Req->ProcessGet(Url);
		}
	}
}

void UBlueprintHttp::HttpPost(UObject* WorldContextObject,
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
	FLatentActionInfo LatentInfo)
{
	UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] HttpPost %s"), *Url);
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] World OK"));
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FBlueprintHttpAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
		{
			UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttp] LatentAction not found => new"));

			// The real log line for server admins
			UE_LOG(LogBlueprintUserMessages, Log, TEXT("[BlueprintHttp] Http POST %s"), *Url);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.bNoFail = true;
			TSharedRef<ABlueprintHttpRequest> Req(World->SpawnActor<ABlueprintHttpRequest>(ABlueprintHttpRequest::StaticClass(), SpawnParams));

			FBlueprintHttpAction* HttpAction = new FBlueprintHttpAction(Req, &Success, &Code, &Body, &Keys, &Values, LatentInfo);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, HttpAction);
			Req->AddUserHeaders(HeaderKeys, HeaderValues);
			Req->ProcessPost(Url, JsonBody);
		}
	}
}

TSharedPtr<FJsonObject> UBlueprintHttp::FlattenJson(const TSharedPtr<FJsonObject>& InObj)
{
	TSharedPtr<FJsonObject> OutObj(new FJsonObject());

	for (auto Pair = InObj->Values.CreateConstIterator(); Pair; ++Pair)
		FlattenJson_Internal(Pair->Value, OutObj, Pair->Key);

	return OutObj;
}

void UBlueprintHttp::FlattenJson_Internal(const TSharedPtr<FJsonValue>& InVal, TSharedPtr<FJsonObject>& OutObj, const FString& Path)
{
	switch (InVal->Type)
	{
		case EJson::None:
		case EJson::Null:
		case EJson::Boolean:
		case EJson::Number:
		case EJson::String:
		{
			OutObj->SetField(Path, InVal);
			return;
		}
		case EJson::Array:
		{
			auto Arr = InVal->AsArray();
			OutObj->SetNumberField(Path + ".length", Arr.Num());
			for (int32 i = 0; i < Arr.Num(); i++)
				FlattenJson_Internal(Arr[i], OutObj, FString::Printf(TEXT("%s.%i"), *Path, i));
			return;
		}
		case EJson::Object:
		{
			for (auto Pair = InVal->AsObject()->Values.CreateConstIterator(); Pair; ++Pair)
				FlattenJson_Internal(Pair->Value, OutObj, Path + "." + Pair->Key);
			return;
		}
	}
}

void UBlueprintHttp::MakeSimpleJson(const TArray<FString>& Keys, const TArray<FString>& Values, FString& Json)
{
	Json = "{";
	for (int32 i = 0; i < Keys.Num(); i++)
	{
		Json += FString((i > 0) ? "," : "")
			+ FormatJsonValue(Keys[i], "string")
			+ ":"
			+ FormatJsonValue(Values.IsValidIndex(i) ? Values[i] : "", "string");
	}
	Json += "}";
}

void UBlueprintHttp::MakeJsonObject(const TArray<FString>& Keys, const TArray<FString>& Values, const TArray<FString>& Types, FString& Json)
{
	Json = "{";
	for (int32 i = 0; i < Keys.Num(); i++)
	{
		Json += FString((i>0) ? "," : "")
			+ FormatJsonValue(Keys[i], "string")
			+ ":"
			+ FormatJsonValue(Values.IsValidIndex(i) ? Values[i] : "", Types.IsValidIndex(i) ? Types[i] : "string");
	}
	Json += "}";
}

void UBlueprintHttp::MakeJsonArray(const TArray<FString>& Values, const TArray<FString>& Types, FString& Json)
{
	Json = "[";
	for (int32 i = 0; i < Values.Num(); i++)
	{
		Json += FString((i>0) ? "," : "")
			+ FormatJsonValue(Values.IsValidIndex(i) ? Values[i] : "", Types.IsValidIndex(i) ? Types[i] : "string");
	}
	Json += "]";
}

FString UBlueprintHttp::FormatJsonValue(const FString& UserValue, const FString& Type)
{
	// the only type that needs reformating is String
	if (Type.ToLower().Equals("string"))
	{
		FString OutString;
		OutString += TEXT("\"");
		for (const TCHAR* Char = *UserValue; *Char != TCHAR('\0'); ++Char)
		{
			switch (*Char)
			{
				case TCHAR('\\'): OutString += TEXT("\\\\"); break;
				case TCHAR('\n'): OutString += TEXT("\\n"); break;
				case TCHAR('\t'): OutString += TEXT("\\t"); break;
				case TCHAR('\b'): OutString += TEXT("\\b"); break;
				case TCHAR('\f'): OutString += TEXT("\\f"); break;
				case TCHAR('\r'): OutString += TEXT("\\r"); break;
				case TCHAR('\"'): OutString += TEXT("\\\""); break;
				default: OutString += *Char;
			}
		}
		OutString += TEXT("\"");
		return OutString;
	}
	return UserValue;
}


//================================================
// Request object
//================================================

ABlueprintHttpRequest::ABlueprintHttpRequest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	Request = FHttpModule::Get().CreateRequest();
	Request->SetHeader("User-Agent", "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Accept", "application/json");
}

void ABlueprintHttpRequest::AddUserHeaders(const TArray<FString>& Keys, const TArray<FString>& Values)
{
	for (int32 i = 0; i < Keys.Num(); i++)
	{
		Request->SetHeader(Keys[i], Values.IsValidIndex(i) ? Values[i] : "");
	}
}

void ABlueprintHttpRequest::ProcessGet(const FString& Url)
{
	UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] CreateRequest GET %s"), *Url);

	Request->SetURL(Url);
	Request->SetVerb("GET");
	Request->OnProcessRequestComplete().BindUObject(this, &ABlueprintHttpRequest::OnResponseReceived);
	Request->ProcessRequest();
}

void ABlueprintHttpRequest::ProcessPost(const FString& Url, const FString& JsonBody)
{
	UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] CreateRequest POST %s"), *Url);

	Request->SetURL(Url);
	Request->SetVerb("POST");
	Request->SetContentAsString(JsonBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ABlueprintHttpRequest::OnResponseReceived);
	Request->ProcessRequest();
}

void ABlueprintHttpRequest::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Happens when eg. hostname doesn't resolve
	if (!Response.IsValid())
	{
		UE_LOG(LogBlueprintUserMessages, Warning, TEXT("[BlueprintHttpRequest] Response is NULL !!"));
		Done = true;
		this->Request.Reset();
		return;
	}

	UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] OnResponseReceived %i"), Response->GetResponseCode());

	Success = bWasSuccessful;
	Code = Response->GetResponseCode();
	Body = Response->GetContentAsString();

	if (bWasSuccessful)
	{
		UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] Was Successful"));

		// JSON Deserializer doesn't handle array roots, so wrap in an object
		if (Body.Trim().StartsWith("["))
			Body = "{\"\":" + Body + "}";

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] Deserialize OK"));

			JsonObject = UBlueprintHttp::FlattenJson(JsonObject);
			UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] Flatten OK"));

			JsonObject->Values.GetKeys(Keys);
			Values.SetNum(Keys.Num());
			for (int32 i = 0; i < Keys.Num(); i++)
			{
				if (!JsonObject->TryGetStringField(Keys[i], Values[i]))
					Values[i] = "";
			}
		}
		else
		{
			UE_LOG(LogBlueprintUserMessages, Warning, TEXT("[BlueprintHttpRequest] Failed to deserialize JSON"));
			Success = false;
		}
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Warning, TEXT("[BlueprintHttpRequest] Request failed (%i)"), Code);
	}

	Done = true;
	this->Request.Reset();	// our reference is no longer necessary
}

void ABlueprintHttpRequest::Cancel()
{
	if (!Done)
	{
		UE_LOG(LogBlueprintUserMessages, DEBUGLOG, TEXT("[BlueprintHttpRequest] Cancelling"));
		Request->CancelRequest();
	}
}
