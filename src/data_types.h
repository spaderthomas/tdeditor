typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef unsigned short uint16;
typedef unsigned char uchar;
typedef uint32_t bool;

typedef struct IVec2 {
	uint32 x;
	uint32 y;
} IVec2;

typedef struct Vec2 {
	float x;
	float y;
} Vec2;

typedef struct Vec3 {
	float x;
	float y;
	float z;
} Vec3;

typedef struct Vertex {
	Vec2 pos;
	Vec3 color;
	Vec2 uv;
} Vertex;
