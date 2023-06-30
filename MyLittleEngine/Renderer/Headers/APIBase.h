#pragma once
#include "../../Utility/Headers/CommonHeader.h"

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch DirectX API errors
        throw std::exception();
    }
}

enum ASSETTYPE
{
    Shader,
    Texture,
    Mesh,

};

typedef struct
{
    Vector4 vertices;
    Vector4 normals;
    Vector4 colors;
    Vector2 uv;

} VertexStruct;

class APIBase
{

public:
    virtual void InitAPI(int width, int height) {}
    virtual void LoadAssetsFromFile(ASSETTYPE type, char* path) {}
    virtual void Render() {}
    virtual void Update() {}
    virtual void End() {}
};