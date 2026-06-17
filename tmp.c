#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef void* ptr;
typedef const char* str;

typedef u8 byte;
typedef u64 usize;
typedef i64 isize;
typedef ptr RawHandle;
typedef enum Status {
    Status_Ok,
    Status_End,
    Status_Invalid,
    Status_NoMemory,
    Status_NotFound,
    Status_PermissionDenied,
    Status_Interrupted,
    Status_Unsupported,
    Status_IoError,
} Status;



#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdlib.h>
#include <string.h>

static void* kek_std_alloc(size_t size) {
	return malloc(size);
}

static void* kek_std_resize(void* oldData, size_t size) {
	return realloc(oldData, size);
}

static void kek_std_free(void* data) {
	free(data);
}

static void* kek_std_mem_copy(void* dest, const void* src, size_t count) {
	return memcpy(dest, src, count);
}

static void* kek_std_mem_set(void* dest, int value, size_t count) {
	return memset(dest, value, count);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

struct Allocator {
    ptr context;
};
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdio.h>

static FILE* kek_std_file_open(const char* path, const char* mode) {
	return fopen(path, mode);
}

static size_t kek_std_file_read(void* data, size_t size, size_t count, FILE* file) {
	return fread(data, size, count, file);
}

static size_t kek_std_file_write(const void* data, size_t size, size_t count, FILE* file) {
	return fwrite(data, size, count, file);
}

static int kek_std_file_flush(FILE* file) {
	return fflush(file);
}

static int kek_std_file_close(FILE* file) {
	return fclose(file);
}

static FILE* kek_std_stdin(void) {
	return stdin;
}

static FILE* kek_std_stdout(void) {
	return stdout;
}

static FILE* kek_std_stderr(void) {
	return stderr;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

typedef enum FileMode {
    FileMode_Read,
    FileMode_Write,
    FileMode_Append,
    FileMode_ReadWrite,
} FileMode;
struct File {
    RawHandle handle;
    bool owned;
};
struct MemoryReader {
    byte* data;
    usize len;
    usize pos;
};
struct MemoryWriter {
    byte* data;
    usize len;
    usize pos;
};
struct String {
    byte* data;
    usize len;
};
struct OwnedString {
    byte* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct StringBuilder {
    byte* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct ByteCursor {
    struct String input;
    usize pos;
    usize line;
    usize column;
};



#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <dirent.h>

static DIR* kek_std_dir_open(const char* path) {
	return opendir(path);
}

static const char* kek_std_dir_read_name(DIR* dir) {
	struct dirent* entry = readdir(dir);
	if (!entry) {
		return 0;
	}
	return entry->d_name;
}

static int kek_std_dir_close(DIR* dir) {
	return closedir(dir);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

struct Directory {
    RawHandle handle;
};
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdlib.h>

static int kek_std_system(const char* command) {
	return system(command);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif


    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    typedef FILE KekFile;
    typedef size_t KekSize;
    typedef int KekStatus;

    struct CPoint {
        int x;
        int y;
    };

    void* kek_alloc(KekSize size) { return malloc(size); }
    void kek_free(void* memory) { free(memory); }
    KekFile* kek_file_open(const char* path, const char* mode) { return fopen(path, mode); }
    KekSize kek_file_read(void* buffer, KekSize size, KekSize count, KekFile* file) { return fread(buffer, size, count, file); }
    KekSize kek_file_write(const void* buffer, KekSize size, KekSize count, KekFile* file) { return fwrite(buffer, size, count, file); }
    KekStatus kek_file_close(KekFile* file) { return fclose(file); }
    void* kek_mem_copy(void* dest, const void* src, KekSize count) { return memcpy(dest, src, count); }
    void* kek_mem_set(void* dest, int value, KekSize count) { return memset(dest, value, count); }
    int kek_str_cmp(const char* left, const char* right) { return strcmp(left, right); }
    void kek_exit(int status) { exit(status); }

    void println(const char* str) {
        printf("%s\n", str);
    }


#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

typedef u8 byte;
typedef u64 usize;
struct __attribute__((packed,aligned(8))) AlignedPacket {
    u8 tag;
    u32 data;
};
struct Container {
    u32 id;
    struct {
        u8 value;
    } Inner;
};
struct __attribute__((packed)) Packet {
    u8 kind;
    u32 id;
};
typedef enum PacketKind {
    PacketKind_None,
    PacketKind_Add=3,
} PacketKind;
typedef union PacketValue {
    u8 kind;
    u32 id;
} PacketValue;


struct Vec2 {
    i32 x;
    i32 y;
};
struct Result__File {
    Status status;
    struct File value;
};
struct Result__usize {
    Status status;
    usize value;
};
struct Span__byte {
    byte* data;
    usize len;
};
struct Slice__byte {
    byte* data;
    usize len;
};
struct Result__OwnedString {
    Status status;
    struct OwnedString value;
};
struct Result__Directory {
    Status status;
    struct Directory value;
};
struct Result__String {
    Status status;
    struct String value;
};
struct GenericBox__u8 {
    u8 value;
};
struct Generics__u8__u16 {
    u8 first;
    u16 second;
};
i32 MyPackage_MyPackageFunc(void);
struct Allocator std_DefaultAllocator(void);
void std_SetBytes(byte* dest,byte value,usize count);
struct Result__File std_FileOpen(str path,FileMode mode);
struct Result__usize File_Read(struct File* this,struct Span__byte out);
struct Result__usize File_Write(struct File* this,struct Slice__byte data);
Status File_Flush(struct File* this);
Status std_ReadFileToOwnedString(str path,struct Allocator allocator,struct OwnedString* out);
Status std_WriteFile(str path,struct String text);
Status File_Close(struct File* this);
struct File std_Stdin(void);
struct File std_Stdout(void);
struct File std_Stderr(void);
Status std_ReadAllToOwnedString(struct File file,struct Allocator allocator,struct OwnedString* out);
struct MemoryReader std_MemoryReaderNew(struct Slice__byte data);
struct MemoryWriter std_MemoryWriterNew(struct Span__byte data);
struct Result__usize MemoryReader_Read(struct MemoryReader* this,struct Span__byte out);
struct Result__usize MemoryWriter_Write(struct MemoryWriter* this,struct Slice__byte data);
usize MemoryWriter_Written(struct MemoryWriter* this);
Status std_WriteByteToMemory(struct MemoryWriter* writer,byte value);
struct String std_StringFromBytes(struct Slice__byte bytes);
struct String std_StringFromCString(str text);
struct Slice__byte String_Bytes(struct String* this);
struct String String_Slice(struct String* this,usize start,usize length);
bool String_Equals(struct String* this,struct String other);
bool String_EqualsCString(struct String* this,str text);
int String_Compare(struct String* this,struct String other);
bool String_EqualsBytes(struct String* this,byte* data,usize len);
struct Result__usize String_FindByte(struct String* this,byte value);
bool String_ContainsByte(struct String* this,byte value);
bool String_StartsWith(struct String* this,struct String prefix);
bool String_EndsWith(struct String* this,struct String suffix);
bool std_IsAsciiSpace(byte c);
bool std_IsAsciiAlpha(byte c);
bool std_IsAsciiDigit(byte c);
bool std_IsAsciiWord(byte c);
bool std_IsAsciiOperator(byte c);
struct String OwnedString_View(struct OwnedString* this);
struct String std_OwnedStringView(struct OwnedString* owned);
Status std_DestroyOwnedString(struct OwnedString* owned);
Status std_CloneString(struct String text,struct Allocator allocator,struct OwnedString* out);
Status std_CloneCString(str text,struct Allocator allocator,struct OwnedString* out);
Status OwnedString_Destroy(struct OwnedString* this);
struct StringBuilder std_StringBuilderNew(struct Allocator allocator);
Status StringBuilder_Destroy(struct StringBuilder* this);
Status StringBuilder_Clear(struct StringBuilder* this);
Status StringBuilder_Reserve(struct StringBuilder* this,usize additional);
struct Result__usize StringBuilder_Write(struct StringBuilder* this,struct Slice__byte data);
Status StringBuilder_WriteByte(struct StringBuilder* this,byte value);
Status StringBuilder_WriteString(struct StringBuilder* this,struct String text);
Status StringBuilder_WriteCString(struct StringBuilder* this,str text);
Status StringBuilder_WriteRepeatByte(struct StringBuilder* this,byte value,usize count);
Status StringBuilder_WriteIndent(struct StringBuilder* this,usize count);
struct String StringBuilder_View(struct StringBuilder* this);
struct Result__OwnedString StringBuilder_Detach(struct StringBuilder* this);
struct Result__OwnedString StringBuilder_ToOwnedString(struct StringBuilder* this);
Status std_WriteStringToBuilder(struct StringBuilder* builder,struct String text);
Status std_WriteSliceToBuilder(struct StringBuilder* builder,struct Slice__byte data);
Status std_DestroyStringBuilder(struct StringBuilder* builder);
struct ByteCursor std_ByteCursorNew(struct String input);
bool ByteCursor_AtEnd(struct ByteCursor* this);
byte ByteCursor_Peek(struct ByteCursor* this);
byte ByteCursor_PeekAt(struct ByteCursor* this,usize offset);
byte ByteCursor_Advance(struct ByteCursor* this);
bool ByteCursor_MatchByte(struct ByteCursor* this,byte value);
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this);
Status std_WriteByteToBuilder(struct StringBuilder* writer,byte value);
Status std_WriteByteToFile(struct File* writer,byte value);
Status std_WriteStringToMemory(struct MemoryWriter* writer,struct String text);
Status std_WriteStringToFile(struct File* writer,struct String text);
Status std_WriteBoolToBuilder(struct StringBuilder* writer,bool value);
Status std_WriteBoolToMemory(struct MemoryWriter* writer,bool value);
Status std_WriteBoolToFile(struct File* writer,bool value);
Status std_FormatU64ToBuilder(struct StringBuilder* writer,u64 value,u8 base);
Status std_FormatI64ToBuilder(struct StringBuilder* writer,i64 value);
Status std_FormatBoolToBuilder(struct StringBuilder* writer,bool value);
struct Result__Directory std_DirectoryOpen(str path);
struct Result__String Directory_ReadName(struct Directory* this);
Status Directory_Close(struct Directory* this);
int std_ProcessRun(str command);
struct Vec2 Vec2_operator_plus_1(struct Vec2* this,struct Vec2 rhs);
struct Vec2 Vec2_operator_minus_1(struct Vec2* this,struct Vec2 rhs);
struct Vec2 Vec2_operator_minus_0(struct Vec2* this);
bool Vec2_operator_equal_1(struct Vec2* this,struct Vec2 rhs);
void Vec2_operator_plus_assign_1(struct Vec2* this,struct Vec2 rhs);
int add(int a,int b);
u8 Packet_Add(struct Packet* this,u8 val);
int main(void);
struct Span__byte std_FixedSpan__byte(byte* data,usize len);
struct Slice__byte std_FixedSlice__byte(byte* data,usize len);
void std_Free__byte(struct Allocator allocator,byte* data,usize count);
byte* std_Alloc__byte(struct Allocator allocator,usize count);
byte* std_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount);
int GenericAdd__int(int left,int right);
u8 MyPackage_PkgGenericEcho__u8(u8 value);
struct Span__byte std_FixedSpan__byte(byte* data,usize len) {
    struct Span__byte out = {.data=0,.len=0};
    out.data=data;
    out.len=len;
    return((out));
}
struct Slice__byte std_FixedSlice__byte(byte* data,usize len) {
    struct Slice__byte out = {.data=0,.len=0};
    out.data=data;
    out.len=len;
    return((out));
}
void std_Free__byte(struct Allocator allocator,byte* data,usize count) {
    if (allocator.context!=0) {
    }
    if (count==0) {
    }
    kek_std_free(data);
}
byte* std_Alloc__byte(struct Allocator allocator,usize count) {
    if (allocator.context!=0) {
    }
    return((kek_std_alloc(count*sizeof(byte))));
}
byte* std_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount) {
    if (allocator.context!=0) {
    }
    if (oldCount==0) {
    }
    return((kek_std_resize(oldData,newCount*sizeof(byte))));
}
int GenericAdd__int(int left,int right) {
    return((left+right));
}
u8 MyPackage_PkgGenericEcho__u8(u8 value) {
    return((value));
}
i32 MyPackage_MyPackageFunc(void) {
    return((69));
}





struct Allocator std_DefaultAllocator(void) {
    return(((struct Allocator){.context=0}));
}




void std_SetBytes(byte* dest,byte value,usize count) {
    kek_std_mem_set(dest,value,count);
}
struct Result__File std_FileOpen(str path,FileMode mode) {
    struct Result__File result = {.status=0,.value={0}};
    str modeText="rb";
    if (mode==FileMode_Write) {
        modeText="wb";
    }
    else {
        if (mode==FileMode_Append) {
            modeText="ab";
        }
        else {
            if (mode==FileMode_ReadWrite) {
                modeText="r+b";
            }
        }
    }
    RawHandle handle=kek_std_file_open(path,modeText);
    if (handle==0) {
        result.status=Status_NotFound;
        return((result));
    }
    struct File file = {.handle=0,.owned=0};
    file.handle=handle;
    file.owned=1;
    result.status=Status_Ok;
    result.value=file;
    return((result));
}
struct Result__usize File_Read(struct File* this,struct Span__byte out) {
    struct Result__usize result = {.status=0,.value=0};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return((result));
    }
    usize read=kek_std_file_read(out.data,1,out.len,this->handle);
    if (read==0) {
        result.status=Status_End;
        return((result));
    }
    result.status=Status_Ok;
    result.value=read;
    return((result));
}
struct Result__usize File_Write(struct File* this,struct Slice__byte data) {
    struct Result__usize result = {.status=0,.value=0};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return((result));
    }
    usize written=kek_std_file_write(data.data,1,data.len,this->handle);
    if (written!=data.len) {
        result.status=Status_IoError;
        result.value=written;
        return((result));
    }
    result.status=Status_Ok;
    result.value=written;
    return((result));
}
Status File_Flush(struct File* this) {
    if (this->handle==0) {
        return((Status_Invalid));
    }
    if (kek_std_file_flush(this->handle)!=0) {
        return((Status_IoError));
    }
    return((Status_Ok));
}
Status std_ReadFileToOwnedString(str path,struct Allocator allocator,struct OwnedString* out) {
    struct Result__File opened=std_FileOpen(path,FileMode_Read);
    if (opened.status!=Status_Ok) {
        return((opened.status));
    }
    struct File file=opened.value;
    Status status=std_ReadAllToOwnedString(file,allocator,out);
    Status closeStatus=File_Close(&file);
    if (status!=Status_Ok) {
        return((status));
    }
    return((closeStatus));
}
Status std_WriteFile(str path,struct String text) {
    struct Result__File opened=std_FileOpen(path,FileMode_Write);
    if (opened.status!=Status_Ok) {
        return((opened.status));
    }
    struct File file=opened.value;
    struct Result__usize write=File_Write(&file,String_Bytes(&text));
    Status closeStatus=File_Close(&file);
    if (write.status!=Status_Ok) {
        return((write.status));
    }
    return((closeStatus));
}
Status File_Close(struct File* this) {
    if (this->handle==0) {
        return((Status_Invalid));
    }
    if (!this->owned) {
        return((Status_Unsupported));
    }
    if (kek_std_file_close(this->handle)!=0) {
        return((Status_IoError));
    }
    this->handle=0;
    this->owned=0;
    return((Status_Ok));
}
struct File std_Stdin(void) {
    struct File file = {.handle=0,.owned=0};
    file.handle=kek_std_stdin();
    file.owned=0;
    return((file));
}
struct File std_Stdout(void) {
    struct File file = {.handle=0,.owned=0};
    file.handle=kek_std_stdout();
    file.owned=0;
    return((file));
}
struct File std_Stderr(void) {
    struct File file = {.handle=0,.owned=0};
    file.handle=kek_std_stderr();
    file.owned=0;
    return((file));
}
Status std_ReadAllToOwnedString(struct File file,struct Allocator allocator,struct OwnedString* out) {
    struct StringBuilder builder=std_StringBuilderNew(allocator);
    byte buffer[(4096)]={0};
    while (1) {
        struct Result__usize read=File_Read(&file,std_FixedSpan__byte(buffer,(sizeof((buffer))/sizeof(((buffer))[0]))));
        if (read.status==Status_End) {
            break;
        }
        if (read.status!=Status_Ok) {
            StringBuilder_Destroy(&builder);
            return((read.status));
        }
        struct Result__usize write=StringBuilder_Write(&builder,std_FixedSlice__byte(buffer,read.value));
        if (write.status!=Status_Ok) {
            StringBuilder_Destroy(&builder);
            return((write.status));
        }
    }
    out->data=builder.data;
    out->len=builder.len;
    out->cap=builder.cap;
    out->allocator=builder.allocator;
    builder.data=0;
    builder.len=0;
    builder.cap=0;
    return((Status_Ok));
}
struct MemoryReader std_MemoryReaderNew(struct Slice__byte data) {
    struct MemoryReader reader = {.data=0,.len=0,.pos=0};
    reader.data=data.data;
    reader.len=data.len;
    reader.pos=0;
    return((reader));
}
struct MemoryWriter std_MemoryWriterNew(struct Span__byte data) {
    struct MemoryWriter writer = {.data=0,.len=0,.pos=0};
    writer.data=data.data;
    writer.len=data.len;
    writer.pos=0;
    return((writer));
}
struct Result__usize MemoryReader_Read(struct MemoryReader* this,struct Span__byte out) {
    struct Result__usize result = {.status=0,.value=0};
    if (this->pos>=this->len) {
        result.status=Status_End;
        return((result));
    }
    usize remaining=this->len-this->pos;
    usize amount=out.len;
    if (amount>remaining) {
        amount=remaining;
    }
    for (usize i=0;i<amount;i++) {
        out.data[(i)]=this->data[(this->pos+i)];
    }
    this->pos+=amount;
    result.status=Status_Ok;
    result.value=amount;
    return((result));
}
struct Result__usize MemoryWriter_Write(struct MemoryWriter* this,struct Slice__byte data) {
    struct Result__usize result = {.status=0,.value=0};
    usize remaining=this->len-this->pos;
    if (data.len>remaining) {
        result.status=Status_NoMemory;
        result.value=this->pos;
        return((result));
    }
    for (usize i=0;i<data.len;i++) {
        this->data[(this->pos+i)]=data.data[(i)];
    }
    this->pos+=data.len;
    result.status=Status_Ok;
    result.value=data.len;
    return((result));
}
usize MemoryWriter_Written(struct MemoryWriter* this) {
    return((this->pos));
}
Status std_WriteByteToMemory(struct MemoryWriter* writer,byte value) {
    byte buffer[(1)]={value};
    struct Result__usize result=MemoryWriter_Write(writer,std_FixedSlice__byte(buffer,1));
    return((result.status));
}
struct String std_StringFromBytes(struct Slice__byte bytes) {
    struct String out = {.data=0,.len=0};
    out.data=bytes.data;
    out.len=bytes.len;
    return((out));
}
struct String std_StringFromCString(str text) {
    struct String out = {.data=0,.len=0};
    usize count=0;
    while (text[(count)]!=0) {
        count+=1;
    }
    out.data=((ptr)((text)));
    out.len=count;
    return((out));
}
struct Slice__byte String_Bytes(struct String* this) {
    struct Slice__byte out = {.data=0,.len=0};
    out.data=this->data;
    out.len=this->len;
    return((out));
}
struct String String_Slice(struct String* this,usize start,usize length) {
    struct String out = {.data=0,.len=0};
    if (start>=this->len) {
        out.data=this->data+this->len;
        out.len=0;
        return((out));
    }
    usize available=this->len-start;
    if (length>available) {
        length=available;
    }
    out.data=this->data+start;
    out.len=length;
    return((out));
}
bool String_Equals(struct String* this,struct String other) {
    if (this->len!=other.len) {
        return((0));
    }
    for (usize i=0;i<this->len;i++) {
        if (this->data[(i)]!=other.data[(i)]) {
            return((0));
        }
    }
    return((1));
}
bool String_EqualsCString(struct String* this,str text) {
    return((String_Equals(this,std_StringFromCString(text))));
}
int String_Compare(struct String* this,struct String other) {
    usize limit=this->len;
    if (other.len<limit) {
        limit=other.len;
    }
    for (usize i=0;i<limit;i++) {
        if (this->data[(i)]<other.data[(i)]) {
            return((-1));
        }
        if (this->data[(i)]>other.data[(i)]) {
            return((1));
        }
    }
    if (this->len<other.len) {
        return((-1));
    }
    if (this->len>other.len) {
        return((1));
    }
    return((0));
}
bool String_EqualsBytes(struct String* this,byte* data,usize len) {
    if (this->len!=len) {
        return((0));
    }
    for (usize i=0;i<len;i++) {
        if (this->data[(i)]!=data[(i)]) {
            return((0));
        }
    }
    return((1));
}
struct Result__usize String_FindByte(struct String* this,byte value) {
    struct Result__usize result = {.status=0,.value=0};
    for (usize i=0;i<this->len;i++) {
        if (this->data[(i)]==value) {
            result.status=Status_Ok;
            result.value=i;
            return((result));
        }
    }
    result.status=Status_NotFound;
    return((result));
}
bool String_ContainsByte(struct String* this,byte value) {
    struct Result__usize found=String_FindByte(this,value);
    return((found.status==Status_Ok));
}
bool String_StartsWith(struct String* this,struct String prefix) {
    if (prefix.len>this->len) {
        return((0));
    }
    for (usize i=0;i<prefix.len;i++) {
        if (this->data[(i)]!=prefix.data[(i)]) {
            return((0));
        }
    }
    return((1));
}
bool String_EndsWith(struct String* this,struct String suffix) {
    if (suffix.len>this->len) {
        return((0));
    }
    usize offset=this->len-suffix.len;
    for (usize i=0;i<suffix.len;i++) {
        if (this->data[(offset+i)]!=suffix.data[(i)]) {
            return((0));
        }
    }
    return((1));
}
bool std_IsAsciiSpace(byte c) {
    return((c==32||c==9||c==10||c==13));
}
bool std_IsAsciiAlpha(byte c) {
    return(((c>=65&&c<=90)||(c>=97&&c<=122)||c==95));
}
bool std_IsAsciiDigit(byte c) {
    return((c>=48&&c<=57));
}
bool std_IsAsciiWord(byte c) {
    return((std_IsAsciiAlpha(c)||std_IsAsciiDigit(c)));
}
bool std_IsAsciiOperator(byte c) {
    return((c==33||c==37||c==38||c==42||c==43||c==45||c==47||c==60||c==61||c==62||c==94||c==124));
}
struct String OwnedString_View(struct OwnedString* this) {
    struct String out = {.data=0,.len=0};
    out.data=this->data;
    out.len=this->len;
    return((out));
}
struct String std_OwnedStringView(struct OwnedString* owned) {
    struct String out = {.data=0,.len=0};
    out.data=owned->data;
    out.len=owned->len;
    return((out));
}
Status std_DestroyOwnedString(struct OwnedString* owned) {
    if (owned->data!=0) {
        std_Free__byte(owned->allocator,owned->data,owned->cap);
    }
    owned->data=0;
    owned->len=0;
    owned->cap=0;
    return((Status_Ok));
}
Status std_CloneString(struct String text,struct Allocator allocator,struct OwnedString* out) {
    byte* data=std_Alloc__byte(allocator,text.len+1);
    if (data==0) {
        return((Status_NoMemory));
    }
    for (usize i=0;i<text.len;i++) {
        data[(i)]=text.data[(i)];
    }
    data[(text.len)]=0;
    out->data=data;
    out->len=text.len;
    out->cap=text.len+1;
    out->allocator=allocator;
    return((Status_Ok));
}
Status std_CloneCString(str text,struct Allocator allocator,struct OwnedString* out) {
    return((std_CloneString(std_StringFromCString(text),allocator,out)));
}
Status OwnedString_Destroy(struct OwnedString* this) {
    if (this->data!=0) {
        std_Free__byte(this->allocator,this->data,this->cap);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return((Status_Ok));
}
struct StringBuilder std_StringBuilderNew(struct Allocator allocator) {
    struct StringBuilder builder = {.data=0,.len=0,.cap=0,.allocator={0}};
    builder.data=0;
    builder.len=0;
    builder.cap=0;
    builder.allocator=allocator;
    return((builder));
}
Status StringBuilder_Destroy(struct StringBuilder* this) {
    if (this->data!=0) {
        std_Free__byte(this->allocator,this->data,this->cap);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return((Status_Ok));
}
Status StringBuilder_Clear(struct StringBuilder* this) {
    this->len=0;
    return((Status_Ok));
}
Status StringBuilder_Reserve(struct StringBuilder* this,usize additional) {
    usize needed=this->len+additional;
    if (needed<=this->cap) {
        return((Status_Ok));
    }
    usize newCap=this->cap;
    if (newCap==0) {
        newCap=16;
    }
    while (newCap<needed) {
        newCap=newCap*2;
    }
    byte* newData=std_Resize__byte(this->allocator,this->data,this->cap,newCap);
    if (newData==0) {
        return((Status_NoMemory));
    }
    this->data=newData;
    this->cap=newCap;
    return((Status_Ok));
}
struct Result__usize StringBuilder_Write(struct StringBuilder* this,struct Slice__byte data) {
    struct Result__usize result = {.status=0,.value=0};
    Status status=StringBuilder_Reserve(this,data.len);
    if (status!=Status_Ok) {
        result.status=status;
        return((result));
    }
    for (usize i=0;i<data.len;i++) {
        this->data[(this->len+i)]=data.data[(i)];
    }
    this->len+=data.len;
    result.status=Status_Ok;
    result.value=data.len;
    return((result));
}
Status StringBuilder_WriteByte(struct StringBuilder* this,byte value) {
    byte buffer[(1)]={value};
    struct Result__usize result=StringBuilder_Write(this,std_FixedSlice__byte(buffer,1));
    return((result.status));
}
Status StringBuilder_WriteString(struct StringBuilder* this,struct String text) {
    struct Result__usize result=StringBuilder_Write(this,String_Bytes(&text));
    return((result.status));
}
Status StringBuilder_WriteCString(struct StringBuilder* this,str text) {
    return((StringBuilder_WriteString(this,std_StringFromCString(text))));
}
Status StringBuilder_WriteRepeatByte(struct StringBuilder* this,byte value,usize count) {
    for (usize i=0;i<count;i++) {
        Status status=StringBuilder_WriteByte(this,value);
        if (status!=Status_Ok) {
            return((status));
        }
    }
    return((Status_Ok));
}
Status StringBuilder_WriteIndent(struct StringBuilder* this,usize count) {
    return((StringBuilder_WriteRepeatByte(this,32,count)));
}
struct String StringBuilder_View(struct StringBuilder* this) {
    struct String out = {.data=0,.len=0};
    out.data=this->data;
    out.len=this->len;
    return((out));
}
struct Result__OwnedString StringBuilder_Detach(struct StringBuilder* this) {
    struct OwnedString out = {.data=0,.len=0,.cap=0,.allocator={0}};
    out.data=this->data;
    out.len=this->len;
    out.cap=this->cap;
    out.allocator=this->allocator;
    this->data=0;
    this->len=0;
    this->cap=0;
    struct Result__OwnedString result = {.status=0,.value={0}};
    result.status=Status_Ok;
    result.value=out;
    return((result));
}
struct Result__OwnedString StringBuilder_ToOwnedString(struct StringBuilder* this) {
    return((StringBuilder_Detach(this)));
}
Status std_WriteStringToBuilder(struct StringBuilder* builder,struct String text) {
    struct Result__usize result=StringBuilder_Write(builder,String_Bytes(&text));
    return((result.status));
}
Status std_WriteSliceToBuilder(struct StringBuilder* builder,struct Slice__byte data) {
    struct Result__usize result=StringBuilder_Write(builder,data);
    return((result.status));
}
Status std_DestroyStringBuilder(struct StringBuilder* builder) {
    return((StringBuilder_Destroy(builder)));
}
struct ByteCursor std_ByteCursorNew(struct String input) {
    struct ByteCursor cursor = {.input={0},.pos=0,.line=0,.column=0};
    cursor.input=input;
    cursor.pos=0;
    cursor.line=1;
    cursor.column=1;
    return((cursor));
}
bool ByteCursor_AtEnd(struct ByteCursor* this) {
    return((this->pos>=this->input.len));
}
byte ByteCursor_Peek(struct ByteCursor* this) {
    if (ByteCursor_AtEnd(this)) {
        return((0));
    }
    return((this->input.data[(this->pos)]));
}
byte ByteCursor_PeekAt(struct ByteCursor* this,usize offset) {
    usize target=this->pos+offset;
    if (target>=this->input.len) {
        return((0));
    }
    return((this->input.data[(target)]));
}
byte ByteCursor_Advance(struct ByteCursor* this) {
    byte value=ByteCursor_Peek(this);
    if (ByteCursor_AtEnd(this)) {
        return((0));
    }
    this->pos+=1;
    if (value==10) {
        this->line+=1;
        this->column=1;
    }
    else {
        this->column+=1;
    }
    return((value));
}
bool ByteCursor_MatchByte(struct ByteCursor* this,byte value) {
    if (ByteCursor_Peek(this)!=value) {
        return((0));
    }
    ByteCursor_Advance(this);
    return((1));
}
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this) {
    while (!ByteCursor_AtEnd(this)&&std_IsAsciiSpace(ByteCursor_Peek(this))) {
        ByteCursor_Advance(this);
    }
    return((Status_Ok));
}




















Status std_WriteByteToBuilder(struct StringBuilder* writer,byte value) {
    byte buffer[(1)]={value};
    struct Result__usize result=StringBuilder_Write(writer,std_FixedSlice__byte(buffer,1));
    return((result.status));
}
Status std_WriteByteToFile(struct File* writer,byte value) {
    byte buffer[(1)]={value};
    struct Result__usize result=File_Write(writer,std_FixedSlice__byte(buffer,1));
    return((result.status));
}
Status std_WriteStringToMemory(struct MemoryWriter* writer,struct String text) {
    struct Result__usize result=MemoryWriter_Write(writer,String_Bytes(&text));
    return((result.status));
}
Status std_WriteStringToFile(struct File* writer,struct String text) {
    struct Result__usize result=File_Write(writer,String_Bytes(&text));
    return((result.status));
}
Status std_WriteBoolToBuilder(struct StringBuilder* writer,bool value) {
    byte trueText[(4)]={116,114,117,101};
    byte falseText[(5)]={102,97,108,115,101};
    if (value) {
        return((std_WriteStringToBuilder(writer,std_StringFromBytes(std_FixedSlice__byte(trueText,(sizeof((trueText))/sizeof(((trueText))[0])))))));
    }
    return((std_WriteStringToBuilder(writer,std_StringFromBytes(std_FixedSlice__byte(falseText,(sizeof((falseText))/sizeof(((falseText))[0])))))));
}
Status std_WriteBoolToMemory(struct MemoryWriter* writer,bool value) {
    byte trueText[(4)]={116,114,117,101};
    byte falseText[(5)]={102,97,108,115,101};
    if (value) {
        return((std_WriteStringToMemory(writer,std_StringFromBytes(std_FixedSlice__byte(trueText,(sizeof((trueText))/sizeof(((trueText))[0])))))));
    }
    return((std_WriteStringToMemory(writer,std_StringFromBytes(std_FixedSlice__byte(falseText,(sizeof((falseText))/sizeof(((falseText))[0])))))));
}
Status std_WriteBoolToFile(struct File* writer,bool value) {
    byte trueText[(4)]={116,114,117,101};
    byte falseText[(5)]={102,97,108,115,101};
    if (value) {
        return((std_WriteStringToFile(writer,std_StringFromBytes(std_FixedSlice__byte(trueText,(sizeof((trueText))/sizeof(((trueText))[0])))))));
    }
    return((std_WriteStringToFile(writer,std_StringFromBytes(std_FixedSlice__byte(falseText,(sizeof((falseText))/sizeof(((falseText))[0])))))));
}
Status std_FormatU64ToBuilder(struct StringBuilder* writer,u64 value,u8 base) {
    byte digits[(16)]={48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70};
    byte buffer[(64)]={0};
    usize len=0;
    u64 radix=base;
    if (radix<2) {
        return((Status_Invalid));
    }
    if (radix>16) {
        return((Status_Invalid));
    }
    if (value==0) {
        return((std_WriteByteToBuilder(writer,48)));
    }
    while (value>0) {
        u64 digit=value%radix;
        buffer[(len)]=digits[(digit)];
        len+=1;
        value=value/radix;
    }
    while (len>0) {
        len-=1;
        Status status=std_WriteByteToBuilder(writer,buffer[(len)]);
        if (status!=Status_Ok) {
            return((status));
        }
    }
    return((Status_Ok));
}
Status std_FormatI64ToBuilder(struct StringBuilder* writer,i64 value) {
    if (value<0) {
        Status status=std_WriteByteToBuilder(writer,45);
        if (status!=Status_Ok) {
            return((status));
        }
        return((std_FormatU64ToBuilder(writer,((u64)((0-value))),10)));
    }
    return((std_FormatU64ToBuilder(writer,((u64)((value))),10)));
}
Status std_FormatBoolToBuilder(struct StringBuilder* writer,bool value) {
    return((std_WriteBoolToBuilder(writer,value)));
}
struct Result__Directory std_DirectoryOpen(str path) {
    struct Result__Directory result = {.status=0,.value={0}};
    RawHandle handle=kek_std_dir_open(path);
    if (handle==0) {
        result.status=Status_NotFound;
        return((result));
    }
    struct Directory directory = {.handle=0};
    directory.handle=handle;
    result.status=Status_Ok;
    result.value=directory;
    return((result));
}
struct Result__String Directory_ReadName(struct Directory* this) {
    struct Result__String result = {.status=0,.value={0}};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return((result));
    }
    str name=kek_std_dir_read_name(this->handle);
    if (name==0) {
        result.status=Status_End;
        return((result));
    }
    result.status=Status_Ok;
    result.value=std_StringFromCString(name);
    return((result));
}
Status Directory_Close(struct Directory* this) {
    if (this->handle==0) {
        return((Status_Invalid));
    }
    if (kek_std_dir_close(this->handle)!=0) {
        return((Status_IoError));
    }
    this->handle=0;
    return((Status_Ok));
}
int std_ProcessRun(str command) {
    return((kek_std_system(command)));
}
struct Vec2 Vec2_operator_plus_1(struct Vec2* this,struct Vec2 rhs) {
    return(((struct Vec2){.x=this->x+rhs.x,.y=this->y+rhs.y}));
}
struct Vec2 Vec2_operator_minus_1(struct Vec2* this,struct Vec2 rhs) {
    return(((struct Vec2){.x=this->x-rhs.x,.y=this->y-rhs.y}));
}
struct Vec2 Vec2_operator_minus_0(struct Vec2* this) {
    return(((struct Vec2){.x=-this->x,.y=-this->y}));
}
bool Vec2_operator_equal_1(struct Vec2* this,struct Vec2 rhs) {
    return((this->x==rhs.x&&this->y==rhs.y));
}
void Vec2_operator_plus_assign_1(struct Vec2* this,struct Vec2 rhs) {
    this->x+=rhs.x;
    this->y+=rhs.y;
}

int add(int a,int b) {
    int sum=a+b;
    return((sum));
}
u8 Packet_Add(struct Packet* this,u8 val) {
    this->kind+=val;
    return((this->kind));
}
int main(void) {
    assert(2+3*4==14);
    assert((2+3)*4==20);
    assert(8-3-2==3);
    assert(1+2<4&&5==5);
    int genericSum=GenericAdd__int(20,22);
    assert(genericSum==42);
    struct GenericBox__u8 genericBox = {.value=0};
    genericBox.value=5;
    assert(genericBox.value==5);
    struct Generics__u8__u16 genericPair = {.first=0,.second=0};
    genericPair.first=7;
    genericPair.second=300;
    assert(genericPair.first==7);
    assert(genericPair.second==300);
    struct Vec2 vecA=(struct Vec2){.x=2,.y=3};
    ptr vecPtr=&vecA;
    struct Vec2* typedVecPtr=&vecA;
    assert(vecPtr!=0);
    assert(typedVecPtr!=0);
    struct Vec2 vecB=(struct Vec2){.y=7,.x=5};
    struct Vec2 vecExpected = {.x=0,.y=0};
    vecExpected.x=7;
    vecExpected.y=10;
    struct Vec2 vecSum=Vec2_operator_plus_1(&vecA,vecB);
    assert(Vec2_operator_equal_1(&vecSum,vecExpected));
    struct Vec2 vecDiff=Vec2_operator_minus_1(&vecB,vecA);
    assert(vecDiff.x==3);
    assert(vecDiff.y==4);
    struct Vec2 vecNeg=Vec2_operator_minus_0(&vecA);
    assert(vecNeg.x==-2);
    assert(vecNeg.y==-3);
    Vec2_operator_plus_assign_1(&vecA,vecB);
    assert(Vec2_operator_equal_1(&vecA,vecExpected));
    u8 packageGeneric=MyPackage_PkgGenericEcho__u8(11);
    assert(packageGeneric==11);
    byte small=7;
    assert(small==7);
    int both=add(40,2);
    int one=add(42,0);
    struct Packet packet = {.kind=4,.id=0};
    assert(packet.kind==4);
    u8 kind=Packet_Add(&packet,3);
    assert(kind==7);
    assert(MyPackage_MyPackageFunc()==69);
    PacketKind tag=PacketKind_Add;
    u8 switchValue=0;
    switch (tag) {
        case PacketKind_None: {
            switchValue=1;
            break;
        }
        case PacketKind_Add: {
            switchValue=2;
            break;
        }
        default: switchValue=3;
    }
    assert(switchValue==2);
    PacketValue value;
    value.kind=kind;
    assert(value.kind==7);
    struct CPoint cPoint=(struct CPoint){.x=9,.y=4};
    assert(cPoint.x==9);
    u8 branchValue=0;
    if (kind==7) {
        branchValue=1;
    }
    else {
        branchValue=2;
    }
    assert(branchValue==1);
    int loopSum=0;
    for (int i=0;i<4;i++) {
        if (i==1) {
            continue;
        }
        if (i==3) {
            break;
        }
        loopSum+=i;
    }
    assert(loopSum==2);
    int whileValue=0;
    while (whileValue<3) {
        whileValue+=1;
    }
    assert(whileValue==3);
    do {
        whileValue-=1;
    } while (whileValue>1);
    assert(whileValue==1);
    u8 castValue=((u8)((whileValue+6)));
    assert(castValue==7);
    int deferValue=0;
    if (1) {
        assert(deferValue==0);
        {
            deferValue+=10;
        }
        deferValue+=5;
    }
    assert(deferValue==15);
    u8 bytes[(4)]={1,2,3,4};
    assert(bytes[(2)]==3);
    u8 bitMask=0xA1;
    assert(bitMask==161);
    u32 hexColor=0xFFAA0012;
    assert(hexColor==0xFFAA0012);
    usize packetSize=sizeof(struct Packet);
    assert(packetSize==5);
    usize alignedSize=_Alignof(struct AlignedPacket);
    assert(alignedSize==8);
    usize fieldOffset=offsetof(struct Packet,id);
    assert(fieldOffset==1);
    assert(_Alignof(struct AlignedPacket)==8);
    u8 testArray[(8)]={0};
    usize arrayLen=(sizeof((testArray))/sizeof(((testArray))[0]));
    assert(arrayLen==8);
    assert((sizeof((bytes))/sizeof(((bytes))[0]))==4);
    u8 matrix[(2)][(3)]={0};
    matrix[(0)][(0)]=1;
    matrix[(1)][(2)]=6;
    assert(matrix[(0)][(0)]==1);
    assert(matrix[(1)][(2)]==6);
    struct Container container = {.id=0,.Inner={0}};
    container.id=42;
    container.Inner.value=7;
    assert(container.id==42);
    assert(container.Inner.value==7);
    u8 unreachableTest=1;
    switch (unreachableTest) {
        case 1: {
            break;
        }
        default: {
            __builtin_unreachable();
        }
    }
    if (0) {
        do { fprintf(stderr, "panic: %s\n", ("This should never happen")); abort(); } while(0);
    }
    ptr mem=malloc(16);
    assert(mem!=0);
    free(mem);
    println("Hello, world!");
    str my_string="Whats uuuuuuuuuuuuuuuuuuuuuup!";
    println(my_string);
    u8 eachArr[(4)]={10,20,30,40};
    int eachSum=0;
    for (size_t __kek_each_idx_0 = 0; __kek_each_idx_0 < (sizeof(eachArr)/sizeof((eachArr)[0])); __kek_each_idx_0++) {
        u8 val = (eachArr)[__kek_each_idx_0];
        eachSum+=val;
    }
    assert(eachSum==100);
    u8 eachIndexed[(4)]={0};
    for (usize idx = 0; idx < (sizeof(eachArr)/sizeof((eachArr)[0])); idx++) {
        u8 val = (eachArr)[idx];
        eachIndexed[(idx)]=val+1;
    }
    assert(eachIndexed[(0)]==11);
    assert(eachIndexed[(1)]==21);
    assert(eachIndexed[(2)]==31);
    assert(eachIndexed[(3)]==41);
    int rangeSum=0;
    for (int n = 0; n < 5; n += 1) {
        rangeSum+=n;
    }
    assert(rangeSum==10);
    int stepSum=0;
    for (int n = 0; n < 10; n += 2) {
        stepSum+=n;
    }
    assert(stepSum==20);
    int mulCheck=0;
    for (int i = 1; i < 4; i += 1) {
        for (int j = 1; j < 4; j += 1) {
            mulCheck+=i*j;
        }
    }
    assert(mulCheck==36);
    int idxTest[(3)]={100,200,300};
    int idxSum=0;
    for (usize i = 0; i < (sizeof(idxTest)/sizeof((idxTest)[0])); i++) {
        int val = (idxTest)[i];
        idxSum+=((int)((i)))*val;
    }
    assert(idxSum==800);
    println("each and range tests passed!");
    return((both+one+kind));
}
