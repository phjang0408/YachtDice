#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "DiceMesh.h"

// 셰이더의 FrameCB와 레이아웃이 같아야 한다 (행렬은 전치해서 넣는다)
struct FrameConstants {
	DirectX::XMFLOAT4X4 viewProj;
	DirectX::XMFLOAT4 eyePos;
	DirectX::XMFLOAT4 lightDir;
	DirectX::XMFLOAT4 dicePos[5];
};

// D3D12로 3D 장면을 그리고, 같은 백버퍼 위에 D3D11On12 + Direct2D로 UI를 그린다.
// 한 프레임 순서: BeginFrame -> DrawTable/DrawDie -> BeginUi -> (D2D 그리기) -> EndFrame
class Renderer {
public:
	void Initialize(HWND hwnd, UINT width, UINT height);
	void Shutdown();
	void Resize(UINT width, UINT height);

	bool BeginFrame(const FrameConstants& frame);	// false면 이번 프레임은 그리지 않는다(최소화 등)
	void DrawTable();
	void DrawDie(const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4& color, float highlight, float keep);
	ID2D1DeviceContext2* BeginUi();
	void EndFrame();

	IDWriteFactory* GetDWriteFactory() const { return dwriteFactory.Get(); }
	ID2D1DeviceContext2* GetD2DContext() const { return d2dContext.Get(); }
	UINT GetWidth() const { return width; }
	UINT GetHeight() const { return height; }

private:
	struct ObjectConstants {	// 셰이더의 ObjectCB (루트 상수로 전달)
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 params;
	};

	static constexpr UINT kBackBufferCount = 2;
	static constexpr DXGI_FORMAT kBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	static constexpr DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D32_FLOAT;

	void CreatePipeline();
	void CreateGeometry();
	void CreateUiDevice();
	void CreateSizeDependentResources();
	void ReleaseSizeDependentResources();
	void DrawMesh(const MeshRange& mesh, const ObjectConstants& constants);
	void WaitForGpu();
	D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle(UINT index) const;

	HWND hwnd = nullptr;
	UINT width = 1;
	UINT height = 1;
	bool minimized = false;
	UINT sampleCount = 1;	// MSAA 샘플 수 (지원되면 4)
	UINT frameIndex = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	UINT rtvDescriptorSize = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[kBackBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> msaaTarget;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue = 0;
	HANDLE fenceEvent = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> frameConstantBuffer;
	void* frameConstantsMapped = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	MeshRange dieMesh;
	MeshRange tableMesh;

	// UI (D3D11On12 + Direct2D + DirectWrite)
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context;
	Microsoft::WRL::ComPtr<ID3D11On12Device> d3d11On12Device;
	Microsoft::WRL::ComPtr<ID2D1Factory3> d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2> d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2> d2dContext;
	Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
	Microsoft::WRL::ComPtr<ID3D11Resource> wrappedBackBuffers[kBackBufferCount];
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargets[kBackBufferCount];
};
