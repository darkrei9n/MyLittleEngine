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

class APIBase
{

public:
    virtual void InitAPI(int width, int height) {}
    virtual void Render() {}
    virtual void Update() {}
    virtual void End() {}
};