#pragma once
/*
   This file has been generated by IDA.
   It contains local type definitions from
   the type library 'server.dll'
*/

struct HSquirrelVM;
struct CallInfo;
struct SQTable;
struct SQString;
struct SQFunctionProto;
struct SQClosure;
struct SQSharedState;
struct StringTable;
struct SQStructInstance;
struct SQStructDef;
struct SQNativeClosure;
struct SQArray;
struct tableNode;
struct SQUserData;

typedef void (*releasehookType)(void* val, int size);

/* 127 */
enum SQObjectType : int
{
	_RT_NULL = 0x1,
	_RT_INTEGER = 0x2,
	_RT_FLOAT = 0x4,
	_RT_BOOL = 0x8,
	_RT_STRING = 0x10,
	_RT_TABLE = 0x20,
	_RT_ARRAY = 0x40,
	_RT_USERDATA = 0x80,
	_RT_CLOSURE = 0x100,
	_RT_NATIVECLOSURE = 0x200,
	_RT_GENERATOR = 0x400,
	OT_USERPOINTER = 0x800,
	_RT_USERPOINTER = 0x800,
	_RT_THREAD = 0x1000,
	_RT_FUNCPROTO = 0x2000,
	_RT_CLASS = 0x4000,
	_RT_INSTANCE = 0x8000,
	_RT_WEAKREF = 0x10000,
	OT_VECTOR = 0x40000,
	SQOBJECT_CANBEFALSE = 0x1000000,
	OT_NULL = 0x1000001,
	OT_BOOL = 0x1000008,
	SQOBJECT_DELEGABLE = 0x2000000,
	SQOBJECT_NUMERIC = 0x4000000,
	OT_INTEGER = 0x5000002,
	OT_FLOAT = 0x5000004,
	SQOBJECT_REF_COUNTED = 0x8000000,
	OT_STRING = 0x8000010,
	OT_ARRAY = 0x8000040,
	OT_CLOSURE = 0x8000100,
	OT_NATIVECLOSURE = 0x8000200,
	OT_ASSET = 0x8000400,
	OT_THREAD = 0x8001000,
	OT_FUNCPROTO = 0x8002000,
	OT_CLAAS = 0x8004000,
	OT_STRUCT = 0x8200000,
	OT_WEAKREF = 0x8010000,
	OT_TABLE = 0xA000020,
	OT_USERDATA = 0xA000080,
	OT_INSTANCE = 0xA008000,
	OT_ENTITY = 0xA400000,
};

/* 156 */
union SQObjectValue
{
	SQString* asString;
	SQTable* asTable;
	SQClosure* asClosure;
	SQFunctionProto* asFuncProto;
	SQStructDef* asStructDef;
	long long as64Integer;
	SQNativeClosure* asNativeClosure;
	SQArray* asArray;
	HSquirrelVM* asThread;
	float asFloat;
	int asInteger;
	SQUserData* asUserdata;
};

/* 160 */
struct SQVector
{
	SQObjectType _Type;
	float x;
	float y;
	float z;
};

/* 128 */
struct SQObject
{
	SQObjectType _Type;
	int structNumber;
	SQObjectValue _VAL;
};

/* 138 */
struct alignas(8) SQString
{
	void* vftable;
	int uiRef;
	int padding;
	SQString* _next_maybe;
	SQSharedState* sharedState;
	int length;
	unsigned char gap_24[4];
	char _hash[8];
	char _val[1];
};

/* 137 */
struct alignas(8) SQTable
{
	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	unsigned char gap_10[8];
	void* pointer_18;
	void* pointer_20;
	void* _sharedState;
	long long field_30;
	tableNode* _nodes;
	int _numOfNodes;
	int size;
	int field_48;
	int _usedNodes;
	unsigned char _gap_50[20];
	int field_64;
	unsigned char _gap_68[80];
};

/* 140 */
struct alignas(8) SQClosure
{
	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	void* pointer_10;
	void* pointer_18;
	void* pointer_20;
	void* sharedState;
	SQObject obj_30;
	SQObject _function;
	SQObject* _outervalues;
	unsigned char gap_58[8];
	unsigned char gap_60[96];
	SQObject* objectPointer_C0;
	unsigned char gap_C8[16];
};

/* 139 */
struct alignas(8) SQFunctionProto
{
	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	unsigned char gap_10[8];
	void* pointer_18;
	void* pointer_20;
	void* sharedState;
	void* pointer_30;
	SQObjectType _fileNameType;
	SQString* _fileName;
	SQObjectType _funcNameType;
	SQString* _funcName;
	SQObject obj_58;
	unsigned char gap_68[12];
	int _stacksize;
	unsigned char gap_78[48];
	int nParameters;
	unsigned char gap_AC[60];
	int nDefaultParams;
	unsigned char gap_EC[200];
};

/* 152 */
struct SQStructDef
{
	void* vtable;
	int uiRef;
	unsigned char padding_C[4];
	unsigned char unknown[24];
	SQSharedState* sharedState;
	SQObjectType _nameType;
	SQString* _name;
	unsigned char gap_38[16];
	SQObjectType _variableNamesType;
	SQTable* _variableNames;
	unsigned char gap_[32];
};

/* 157 */
struct alignas(8) SQNativeClosure
{
	void* vftable;
	int uiRef;
	unsigned char gap_C[4];
	long long value_10;
	long long value_18;
	long long value_20;
	SQSharedState* sharedState;
	char unknown_30;
	unsigned char padding_34[7];
	long long value_38;
	long long value_40;
	long long value_48;
	long long value_50;
	long long value_58;
	SQObjectType _nameType;
	SQString* _name;
	long long value_70;
	long long value_78;
	unsigned char justInCaseGap_80[300];
};

/* 162 */
struct SQArray
{
	void* vftable;
	int uiRef;
	unsigned char gap_24[36];
	SQObject* _values;
	int _usedSlots;
	int _allocated;
};

/* 129 */
struct alignas(8) HSquirrelVM
{
	void* vftable;
	int uiRef;
	unsigned char gap_8[12];
	void* _toString;
	void* _roottable_pointer;
	void* pointer_28;
	CallInfo* ci;
	CallInfo* _callstack;
	int _callsstacksize;
	int _stackbase;
	SQObject* _stackOfCurrentFunction;
	SQSharedState* sharedState;
	void* pointer_58;
	void* pointer_60;
	int _top;
	SQObject* _stack;
	unsigned char gap_78[8];
	SQObject* _vargvstack;
	unsigned char gap_88[8];
	SQObject temp_reg;
	unsigned char gapA0[8];
	void* pointer_A8;
	unsigned char gap_B0[8];
	SQObject _roottable_object;
	SQObject _lasterror;
	SQObject _errorHandler;
	long long field_E8;
	int traps;
	unsigned char gap_F4[12];
	int _nnativecalls;
	int _suspended;
	int _suspended_root;
	int _callstacksize;
	int _suspended_target;
	int trapAmount;
	int _suspend_varargs;
	int unknown_field_11C;
	SQObject object_120;
};

/* 150 */
struct SQStructInstance
{
	void* vftable;
	unsigned char gap_8[16];
	void* pointer_18;
	unsigned char gap_20[8];
	SQSharedState* _sharedState;
	unsigned char gap[8];
	SQObject data[20];
};

/* 148 */
struct SQSharedState
{
	unsigned char gap_0[72];
	void* unknown;
	unsigned char gap_50[16344];
	SQObjectType _unknownTableType00;
	long long _unknownTableValue00;
	unsigned char gap_4038[16];
	StringTable* _stringTable;
	unsigned char gap_4050[32];
	SQObjectType _unknownTableType0;
	long long _unknownTableValue0;
	SQObjectType _unknownObjectType1;
	long long _unknownObjectValue1;
	unsigned char gap_4090[8];
	SQObjectType _unknownArrayType2;
	long long _unknownArrayValue2;
	SQObjectType _gobalsArrayType;
	SQStructInstance* _globalsArray;
	unsigned char gap_40B8[16];
	SQObjectType _nativeClosuresType;
	SQTable* _nativeClosures;
	SQObjectType _typedConstantsType;
	SQTable* _typedConstants;
	SQObjectType _untypedConstantsType;
	SQTable* _untypedConstants;
	SQObjectType _globalsMaybeType;
	SQTable* _globals;
	SQObjectType _functionsType;
	SQTable* _functions;
	SQObjectType _structsType;
	SQTable* _structs;
	SQObjectType _typeDefsType;
	SQTable* _typeDefs;
	SQObjectType unknownTableType;
	SQTable* unknownTable;
	SQObjectType _squirrelFilesType;
	SQTable* _squirrelFiles;
	unsigned char gap_4158[80];
	SQObjectType _nativeClosures2Type;
	SQTable* _nativeClosures2;
	SQObjectType _entityTypesMaybeType;
	SQTable* _entityTypesMaybe;
	SQObjectType unknownTable2Type;
	SQTable* unknownTable2;
	unsigned char gap_41D8[72];
	SQObjectType _compilerKeywordsType;
	SQTable* _compilerKeywords;
	HSquirrelVM* _currentThreadMaybe;
	unsigned char gap_4238[8];
	SQObjectType unknownTable3Type;
	SQTable* unknownTable3;
	unsigned char gap_4250[16];
	SQObjectType unknownThreadType;
	SQTable* unknownThread;
	SQObjectType _tableNativeFunctionsType;
	SQTable* _tableNativeFunctions;
	SQObjectType _unknownTableType4;
	long long _unknownObjectValue4;
	SQObjectType _unknownObjectType5;
	long long _unknownObjectValue5;
	SQObjectType _unknownObjectType6;
	long long _unknownObjectValue6;
	SQObjectType _unknownObjectType7;
	long long _unknownObjectValue7;
	SQObjectType _unknownObjectType8;
	long long _unknownObjectValue8;
	SQObjectType _unknownObjectType9;
	long long _unknownObjectValue9;
	SQObjectType _unknownObjectType10;
	long long _unknownObjectValue10;
	SQObjectType _unknownObjectType11;
	long long _unknownObjectValue11;
	SQObjectType _unknownObjectType12;
	long long _unknownObjectValue12;
	SQObjectType _unknownObjectType13;
	long long _unknownObjectValue13;
	SQObjectType _unknownObjectType14;
	long long _unknownObjectValue14;
	SQObjectType _unknownObjectType15;
	long long _unknownObjectValue15;
	unsigned char gap_4340[16];
	void* printFunction;
	unsigned char gap_4358[16];
	void* logEntityFunction;
	unsigned char gap_4370[40];
	SQObjectType _waitStringType;
	SQString* _waitStringValue;
	SQObjectType _SpinOffAndWaitForStringType;
	SQString* _SpinOffAndWaitForStringValue;
	SQObjectType _SpinOffAndWaitForSoloStringType;
	SQString* _SpinOffAndWaitForSoloStringValue;
	SQObjectType _SpinOffStringType;
	SQString* _SpinOffStringValue;
	SQObjectType _SpinOffDelayedStringType;
	SQString* _SpinOffDelayedStringValue;
	unsigned char gap_43E8[8];
	bool enableDebugInfo; // functionality stripped
	unsigned char gap_43F1[23];
};

/* 165 */
struct tableNode
{
	SQObject val;
	SQObject key;
	tableNode* next;
};

/* 136 */
struct alignas(8) CallInfo
{
	long long ip;
	SQObject* _literals;
	SQObject obj10;
	SQObject closure;
	int _etraps[4];
	int _root;
	short _vargs_size;
	short _vargs_base;
	unsigned char gap[16];
};

/* 149 */
struct StringTable
{
	unsigned char gap_0[12];
	int _numofslots;
	unsigned char gap_10[200];
};

/* 141 */
struct alignas(8) SQStackInfos
{
	char* _name;
	char* _sourceName;
	int _line;
};

/* 151 */
struct alignas(4) SQInstruction
{
	int op;
	int arg1;
	int output;
	short arg2;
	short arg3;
};

/* 154 */
struct SQLexer
{
	unsigned char gap_0[112];
};

/* 153 */
struct SQCompiler
{
	unsigned char gap_0[4];
	int _token;
	unsigned char gap_8[8];
	SQObject object_10;
	SQLexer lexer;
	unsigned char gap_90[752];
	HSquirrelVM* sqvm;
	unsigned char gap_288[8];
};

/* 155 */
struct CSquirrelVM
{
	unsigned char gap_0[8];
	HSquirrelVM* sqvm;
	unsigned char gap_10[44];
	int loadEnumFromFileMaybe;
	unsigned char gap_40[200];
};

struct SQUserData
{
	void* vftable;
	int uiRef;
	char gap_12[4];
	long long unknown_10;
	long long unknown_18;
	long long unknown_20;
	long long sharedState;
	long long unknown_30;
	int size;
	char padding1[4];
	releasehookType releaseHook;
	long long typeId;
	char data[1];
};
