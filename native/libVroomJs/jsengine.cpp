
#include <cstring>
#include <iostream>
#include "vroomjs.h"

long js_mem_debug_engine_count;

extern "C" jsvalue CALLINGCONVENTION jsvalue_alloc_array(const int32_t length);

static const int Mega = 1024 * 1024;


static Handle<Value> managed_prop_get(Local<String> name, const AccessorInfo& info)
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_prop_get" << std::endl;
#endif
    HandleScope scope;
    
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
    return scope.Close(ref->GetPropertyValue(name));
}

static Handle<Value> managed_prop_set(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_prop_set" << std::endl;
#endif
    HandleScope scope;
    
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
	if (ref == NULL) {
		Local<Value> result;
		return scope.Close(result);
	}
    return scope.Close(ref->SetPropertyValue(name, value));
}

static Handle<Boolean> managed_prop_delete(Local<String> name, const AccessorInfo& info)
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_prop_delete" << std::endl;
#endif
	 HandleScope scope;
    
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
    return scope.Close(ref->DeleteProperty(name));
}

static Handle<Array> managed_prop_enumerate(const AccessorInfo& info) 
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_prop_enumerate" << std::endl;
#endif
	 HandleScope scope;
    
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
    return scope.Close(ref->EnumerateProperties());
}

static Handle<Value> managed_call(const Arguments& args)
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_call" << std::endl;
#endif
    HandleScope scope;
    
    Local<Object> self = args.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
    return scope.Close(ref->Invoke(args));
}

Handle<Value> managed_valueof(const Arguments& args) {
#ifdef DEBUG_TRACE_API
		std::cout << "managed_valueof" << std::endl;
#endif
    HandleScope scope;
    
    Local<Object> self = args.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
	return scope.Close(ref->GetValueOf());
}

JsEngine* JsEngine::New(int32_t max_young_space = -1, int32_t max_old_space = -1)
{
	JsEngine* engine = new JsEngine();
    if (engine != NULL) 
	{            
		engine->isolate_ = Isolate::New();
		engine->isolate_->Enter();
		
		if (max_young_space > 0 && max_old_space > 0) {
			v8::ResourceConstraints constraints;
			constraints.set_max_young_space_size(max_young_space * Mega);
			constraints.set_max_old_space_size(max_old_space * Mega);
		
			v8::SetResourceConstraints(&constraints);
		}

		engine->isolate_->Exit();

		Locker locker(engine->isolate_);
		Isolate::Scope isolate_scope(engine->isolate_);
		HandleScope scope;      

		// Setup the template we'll use for all managed object references.
        Handle<FunctionTemplate> fo = FunctionTemplate::New(NULL);
		Handle<ObjectTemplate> obj_template = fo->InstanceTemplate();
    	obj_template->SetInternalFieldCount(1);
        obj_template->SetNamedPropertyHandler(
			managed_prop_get, 
			managed_prop_set, 
			NULL, 
			managed_prop_delete, 
			managed_prop_enumerate);
        obj_template->SetCallAsFunctionHandler(managed_call);
        engine->managed_template_ = new Persistent<FunctionTemplate>(Persistent<FunctionTemplate>::New(fo));

		Persistent<FunctionTemplate> fp = Persistent<FunctionTemplate>::New(FunctionTemplate::New(managed_valueof));
		engine->valueof_function_template_ = new Persistent<FunctionTemplate>(fp);
		
		engine->global_context_ = new Persistent<Context>(Context::New());
		(*engine->global_context_)->Enter();

		fo->PrototypeTemplate()->Set(String::New("valueOf"), fp->GetFunction());

		(*engine->global_context_)->Exit();
	}
	return engine;
}

Persistent<Script> *JsEngine::CompileScript(const uint16_t* str, const uint16_t *resourceName, jsvalue *error) {
	Locker locker(isolate_);
	Isolate::Scope isolate_scope(isolate_);
	
    HandleScope scope;
	TryCatch trycatch;
		
	(*global_context_)->Enter();

	Handle<String> source = String::New(str);
	Handle<Script> script;

	if (resourceName != NULL) {
		Handle<String> name = String::New(resourceName);
		script = Script::New(source, name);  
	} else {
		script = Script::New(source);  
	}

	if (script.IsEmpty()) {
		*error = ErrorFromV8(trycatch);
	}
	
	(*global_context_)->Exit();
	
	Persistent<Script> *pScript = new Persistent<Script>(Persistent<Script>::New(script));

	return pScript;
}


void JsEngine::TerminateExecution() 
{
	V8::TerminateExecution(isolate_);
}

void JsEngine::DumpHeapStats() 
{
	Locker locker(isolate_);
    	Isolate::Scope isolate_scope(isolate_);

	// gc first.
	while(!V8::IdleNotification()) {};
	
	HeapStatistics stats;
	isolate_->GetHeapStatistics(&stats);
	std::wcout << "Heap size limit " << (stats.heap_size_limit() / Mega) << std::endl;
	std::wcout << "Total heap size " << (stats.total_heap_size() / Mega) << std::endl;
	std::wcout << "Heap size executable " << (stats.total_heap_size_executable() / Mega) << std::endl;
	std::wcout << "Total physical size " << (stats.total_physical_size() / Mega) << std::endl;
	std::wcout << "Used heap size " << (stats.used_heap_size() / Mega) << std::endl;
}

void JsEngine::Dispose()
{
	if (isolate_ != NULL) {
		isolate_->Enter();

		managed_template_->Dispose();
		delete managed_template_;
		managed_template_ = NULL;
	
		valueof_function_template_->Dispose();
		delete valueof_function_template_;
		valueof_function_template_ = NULL;

		global_context_->Dispose();            
    	delete global_context_;
		global_context_ = NULL;

		isolate_->Exit();
		isolate_->Dispose();
		isolate_ = NULL;
	    keepalive_remove_ = NULL;
		keepalive_get_property_value_ = NULL;
		keepalive_set_property_value_ = NULL;
		keepalive_valueof_ = NULL;
		keepalive_invoke_ = NULL;
		keepalive_delete_property_ = NULL;
		keepalive_enumerate_properties_ = NULL;
	}
}

void JsEngine::DisposeObject(Persistent<Object>* obj)
{
    Locker locker(isolate_);
    Isolate::Scope isolate_scope(isolate_);
    
	obj->Dispose(isolate_);
}

jsvalue JsEngine::ErrorFromV8(TryCatch& trycatch)
{
    jsvalue v;

    HandleScope scope;
    
    Local<Value> exception = trycatch.Exception();
	    
	v.type = JSVALUE_TYPE_UNKNOWN_ERROR;
	v.value.str = 0;
    v.length = 0;

	// If this is a managed exception we need to place its ID inside the jsvalue
    // and set the type JSVALUE_TYPE_MANAGED_ERROR to make sure the CLR side will
    // throw on it.

    if (exception->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(exception);
        if (obj->InternalFieldCount() == 1) {
			Local<External> wrap = Local<External>::Cast(obj->GetInternalField(0));
			ManagedRef* ref = (ManagedRef*)wrap->Value();
	        v.type = JSVALUE_TYPE_MANAGED_ERROR;
            v.length = ref->Id();
			return v;
		}
	}

	jserror *error = new jserror();
	memset(error, 0, sizeof(jserror));
	
	Local<Message> message = trycatch.Message();

	if (!message.IsEmpty()) {
		error->line = message->GetLineNumber();
		error->column = message->GetStartColumn();
		error->resource = AnyFromV8(message->GetScriptResourceName());
		error->message = AnyFromV8(message->Get());
	}
	 if (exception->IsObject()) {
        Local<Object> obj2 = Local<Object>::Cast(exception);
		error->type = AnyFromV8(obj2->GetConstructorName());
	 }

	error->exception = AnyFromV8(exception);
	v.type = JSVALUE_TYPE_ERROR;
	v.value.ptr = error;
    
	return v;
}
    
jsvalue JsEngine::StringFromV8(Handle<Value> value)
{
    jsvalue v;
    
    Local<String> s = value->ToString();
    v.length = s->Length();
    v.value.str = new uint16_t[v.length+1];
    if (v.value.str != NULL) {
        s->Write(v.value.str);
        v.type = JSVALUE_TYPE_STRING;
    }

    return v;
}   

jsvalue JsEngine::WrappedFromV8(Handle<Object> obj)
{
    jsvalue v;
       
	if (js_object_marshal_type == JSOBJECT_MARSHAL_TYPE_DYNAMIC) {
		v.type = JSVALUE_TYPE_WRAPPED;
		v.length = 0;
        // A Persistent<Object> is exactly the size of an IntPtr, right?
		// If not we're in deep deep trouble (on IA32 and AMD64 should be).
		// We should even cast it to void* because C++ doesn't allow to put
		// it in a union: going scary and scarier here.    
		v.value.ptr = new Persistent<Object>(Persistent<Object>::New(obj));
	} else {
		v.type = JSVALUE_TYPE_DICT;
		Local<Array> names = obj->GetOwnPropertyNames();
		v.length = names->Length();
		jsvalue* values = new jsvalue[v.length * 2];
		if (values != NULL) {
			for(int i = 0; i < v.length; i++) {
				int indx = (i * 2);
				Local<Value> key = names->Get(i);
				values[indx] = AnyFromV8(key);
				values[indx+1] = AnyFromV8(obj->Get(key));
			}
			v.value.arr = values;
		}
	}

	return v;
} 

jsvalue JsEngine::ManagedFromV8(Handle<Object> obj)
{
    jsvalue v;
    
	Local<External> wrap = Local<External>::Cast(obj->GetInternalField(0));
    ManagedRef* ref = (ManagedRef*)wrap->Value();
	v.type = JSVALUE_TYPE_MANAGED;
    v.length = ref->Id();
    v.value.str = 0;

    return v;
}
    
jsvalue JsEngine::AnyFromV8(Handle<Value> value, Handle<Object> thisArg)
{
    jsvalue v;
    
    // Initialize to a generic error.
    v.type = JSVALUE_TYPE_UNKNOWN_ERROR;
    v.length = 0;
    v.value.str = 0;
    
    if (value->IsNull() || value->IsUndefined()) {
        v.type = JSVALUE_TYPE_NULL;
    }                
    else if (value->IsBoolean()) {
        v.type = JSVALUE_TYPE_BOOLEAN;
        v.value.i32 = value->BooleanValue() ? 1 : 0;
    }
    else if (value->IsInt32()) {
        v.type = JSVALUE_TYPE_INTEGER;
        v.value.i32 = value->Int32Value();            
    }
    else if (value->IsUint32()) {
        v.type = JSVALUE_TYPE_INDEX;
        v.value.i64 = value->Uint32Value();            
    }
    else if (value->IsNumber()) {
        v.type = JSVALUE_TYPE_NUMBER;
        v.value.num = value->NumberValue();
    }
    else if (value->IsString()) {
        v = StringFromV8(value);
    }
    else if (value->IsDate()) {
        v.type = JSVALUE_TYPE_DATE;
        v.value.num = value->NumberValue();
    }
    else if (value->IsArray()) {
        Handle<Array> object = Handle<Array>::Cast(value->ToObject());
        v.length = object->Length();
        jsvalue* array = new jsvalue[v.length];
        if (array != NULL) {
            for(int i = 0; i < v.length; i++) {
                array[i] = AnyFromV8(object->Get(i));
            }
            v.type = JSVALUE_TYPE_ARRAY;
            v.value.arr = array;
        }
    }
    else if (value->IsFunction()) {
		Handle<Function> function = Handle<Function>::Cast(value);
		jsvalue* array = new jsvalue[2];
        if (array != NULL) { 
			array[0].value.ptr = new Persistent<Object>(Persistent<Function>::New(function));
			array[0].length = 0;
			array[0].type = JSVALUE_TYPE_WRAPPED;
			if (!thisArg.IsEmpty()) {
				array[1].value.ptr = new Persistent<Object>(Persistent<Object>::New(thisArg));
				array[1].length = 0;
				array[1].type = JSVALUE_TYPE_WRAPPED;
			} else {
				array[1].value.ptr = NULL;
				array[1].length = 0;
				array[1].type = JSVALUE_TYPE_NULL;
			}
	        v.type = JSVALUE_TYPE_FUNCTION;
            v.value.arr = array;
        }
    }
    else if (value->IsObject()) {
        Handle<Object> obj = Handle<Object>::Cast(value);
        if (obj->InternalFieldCount() == 1)
            v = ManagedFromV8(obj);
        else
            v = WrappedFromV8(obj);
    }

    return v;
}

jsvalue JsEngine::ArrayFromArguments(const Arguments& args)
{
    jsvalue v = jsvalue_alloc_array(args.Length());
    Local<Object> thisArg = args.Holder();

    for (int i=0 ; i < v.length ; i++) {
        v.value.arr[i] = AnyFromV8(args[i], thisArg);
    }
    
    return v;
}

static void managed_destroy(Persistent<Value> object, void* parameter)
{
#ifdef DEBUG_TRACE_API
		std::cout << "managed_destroy" << std::endl;
#endif
    HandleScope scope;
    
    Persistent<Object> self = Persistent<Object>::Cast(object);
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	ManagedRef* ref = (ManagedRef*)wrap->Value();
    delete ref;
    object.Dispose();
}

Handle<Value> JsEngine::AnyToV8(jsvalue v, int32_t contextId)
{
	if (v.type == JSVALUE_TYPE_EMPTY) {
		return Handle<Value>();
	}
	if (v.type == JSVALUE_TYPE_NULL) {
        return Null();
    }
    if (v.type == JSVALUE_TYPE_BOOLEAN) {
        return Boolean::New(v.value.i32);
    }
    if (v.type == JSVALUE_TYPE_INTEGER) {
        return Int32::New(v.value.i32);
    }
    if (v.type == JSVALUE_TYPE_NUMBER) {
        return Number::New(v.value.num);
    }
    if (v.type == JSVALUE_TYPE_STRING) {
        return String::New(v.value.str);
    }
    if (v.type == JSVALUE_TYPE_DATE) {
        return Date::New(v.value.num);
    }
	
    // Arrays are converted to JS native arrays.
    
    if (v.type == JSVALUE_TYPE_ARRAY) {
        Local<Array> a = Array::New(v.length);
        for(int i = 0; i < v.length; i++) {
            a->Set(i, AnyToV8(v.value.arr[i], contextId));
        }
        return a;        
    }
        
    // This is an ID to a managed object that lives inside the JsContext keep-alive
    // cache. We just wrap it and the pointer to the engine inside an External. A
    // managed error is still a CLR object so it is wrapped exactly as a normal
    // managed object.
    if (v.type == JSVALUE_TYPE_MANAGED || v.type == JSVALUE_TYPE_MANAGED_ERROR) {
		ManagedRef* ref = new ManagedRef(this, contextId, v.length);
		Local<Object> object = (*(managed_template_))->InstanceTemplate()->NewInstance();
		if (object.IsEmpty()) {
			return Null();
		}
		
		Persistent<Object> persistent = Persistent<Object>::New(object);
		persistent->SetInternalField(0, External::New(ref));
		persistent.MakeWeak(NULL, managed_destroy);
        return persistent;
    }

    return Null();
}

int32_t JsEngine::ArrayToV8Args(jsvalue value, int32_t contextId, Handle<Value> preallocatedArgs[])
{
    if (value.type != JSVALUE_TYPE_ARRAY)
        return -1;
        
    for (int i=0 ; i < value.length ; i++) {
        preallocatedArgs[i] = AnyToV8(value.value.arr[i], contextId);
    }
    
    return value.length;
}
