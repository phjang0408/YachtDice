#include "Renderer.h"
#include "Shaders.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace {
	const float kClearColor[4] = { 0.02f, 0.03f, 0.04f, 1.0f };

	void ThrowIfFailed(HRESULT hr, const char* what) {
		if (FAILED(hr)) {
			char buf[256];
			sprintf_s(buf, "%s failed (HRESULT 0x%08X)", what, static_cast<unsigned>(hr));
			throw std::runtime_error(buf);
		}
	}

	D3D12_RESOURCE_BARRIER Transition(ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
		D3D12_RESOURCE_BARRIER b{};
		b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		b.Transition.pResource = resource;
		b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		b.Transition.StateBefore = before;
		b.Transition.StateAfter = after;
		return b;
	}

	D3D12_RESOURCE_DESC BufferDesc(UINT64 size) {
		D3D12_RESOURCE_DESC d{};
		d.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		d.Width = size;
		d.Height = 1;
		d.DepthOrArraySize = 1;
		d.MipLevels = 1;
		d.SampleDesc.Count = 1;
		d.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		return d;
	}

	// 크기가 작은 정적 데이터라 기본 힙 복사 없이 업로드 힙에 바로 둔다
	ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, UINT64 size, const void* data) {
		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC desc = BufferDesc(size);
		ComPtr<ID3D12Resource> buffer;
		ThrowIfFailed(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer)), "CreateCommittedResource(upload)");
		if (data) {
			void* mapped = nullptr;
			D3D12_RANGE noRead{ 0, 0 };
			ThrowIfFailed(buffer->Map(0, &noRead, &mapped), "Map");
			memcpy(mapped, data, static_cast<size_t>(size));
			buffer->Unmap(0, nullptr);
		}
		return buffer;
	}

	ComPtr<ID3DBlob> CompileShader(const char* entry, const char* target) {
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ComPtr<ID3DBlob> code, errors;
		HRESULT hr = D3DCompile(g_ShaderSource, sizeof(g_ShaderSource) - 1, "Shaders.hlsl",
			nullptr, nullptr, entry, target, flags, 0, &code, &errors);
		if (FAILED(hr)) {
			std::string msg = std::string("Shader compile failed (") + entry + ")";
			if (errors) msg += "\n" + std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize());
			throw std::runtime_error(msg);
		}
		return code;
	}
}

void Renderer::Initialize(HWND window, UINT w, UINT h) {
	hwnd = window;
	width = (std::max)(w, 1u);
	height = (std::max)(h, 1u);

	UINT factoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
		debug->EnableDebugLayer();
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)), "CreateDXGIFactory2");

	// 하드웨어 장치가 없으면 WARP(소프트웨어)로 대체
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
		ComPtr<IDXGIAdapter> warp;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)), "EnumWarpAdapter");
		ThrowIfFailed(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "D3D12CreateDevice");
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)), "CreateCommandQueue");

	// 4x MSAA 지원 여부 확인 (컬러/깊이 둘 다)
	auto supports4x = [&](DXGI_FORMAT format) {
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms{};
		ms.Format = format;
		ms.SampleCount = 4;
		return SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms, sizeof(ms)))
			&& ms.NumQualityLevels > 0;
	};
	sampleCount = (supports4x(kBackBufferFormat) && supports4x(kDepthFormat)) ? 4 : 1;

	DXGI_SWAP_CHAIN_DESC1 scDesc{};
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = kBackBufferFormat;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = kBackBufferCount;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(queue.Get(), hwnd, &scDesc, nullptr, nullptr, &swapChain1), "CreateSwapChainForHwnd");
	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	ThrowIfFailed(swapChain1.As(&swapChain), "IDXGISwapChain3");

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = kBackBufferCount + 1;	// 백버퍼 + MSAA 타깃
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)), "CreateDescriptorHeap(RTV)");
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)), "CreateDescriptorHeap(DSV)");

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "CreateCommandAllocator");
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)), "CreateCommandList");
	cmdList->Close();

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence");
	fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (!fenceEvent) throw std::runtime_error("CreateEvent failed");

	CreatePipeline();
	CreateGeometry();
	CreateUiDevice();
	CreateSizeDependentResources();
}

void Renderer::CreatePipeline() {
	// b0: 프레임 상수(루트 CBV), b1: 오브젝트 상수(루트 상수 24개)
	D3D12_ROOT_PARAMETER params[2]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.ShaderRegister = 0;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	params[1].Constants.ShaderRegister = 1;
	params[1].Constants.Num32BitValues = sizeof(ObjectConstants) / 4;
	params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc{};
	rsDesc.NumParameters = 2;
	rsDesc.pParameters = params;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> blob, error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), "D3D12SerializeRootSignature");
	ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "CreateRootSignature");

	ComPtr<ID3DBlob> vs = CompileShader("VSMain", "vs_5_0");
	ComPtr<ID3DBlob> ps = CompileShader("PSMain", "ps_5_0");

	const D3D12_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
	pso.pRootSignature = rootSignature.Get();
	pso.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	pso.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	pso.InputLayout = { layout, _countof(layout) };
	pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pso.RasterizerState.DepthClipEnable = TRUE;
	pso.RasterizerState.MultisampleEnable = sampleCount > 1;
	pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pso.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	pso.DepthStencilState.DepthEnable = TRUE;
	pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pso.SampleMask = UINT_MAX;
	pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso.NumRenderTargets = 1;
	pso.RTVFormats[0] = kBackBufferFormat;
	pso.DSVFormat = kDepthFormat;
	pso.SampleDesc.Count = sampleCount;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState)), "CreateGraphicsPipelineState");
}

void Renderer::CreateGeometry() {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	BuildSceneMeshes(vertices, indices, dieMesh, tableMesh);

	const UINT vbSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
	const UINT ibSize = static_cast<UINT>(indices.size() * sizeof(uint16_t));
	vertexBuffer = CreateUploadBuffer(device.Get(), vbSize, vertices.data());
	indexBuffer = CreateUploadBuffer(device.Get(), ibSize, indices.data());

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vbSize;
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = ibSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// 상수 버퍼는 256바이트 정렬, 계속 매핑해 둔다
	frameConstantBuffer = CreateUploadBuffer(device.Get(), 256, nullptr);
	D3D12_RANGE noRead{ 0, 0 };
	ThrowIfFailed(frameConstantBuffer->Map(0, &noRead, &frameConstantsMapped), "Map(frame CB)");
}

void Renderer::CreateUiDevice() {
	IUnknown* queues[] = { queue.Get() };
	ThrowIfFailed(D3D11On12CreateDevice(device.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
		queues, 1, 0, &d3d11Device, &d3d11Context, nullptr), "D3D11On12CreateDevice");
	ThrowIfFailed(d3d11Device.As(&d3d11On12Device), "ID3D11On12Device");

	D2D1_FACTORY_OPTIONS options{};
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &options,
		reinterpret_cast<void**>(d2dFactory.GetAddressOf())), "D2D1CreateFactory");
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(d3d11On12Device.As(&dxgiDevice), "IDXGIDevice");
	ThrowIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice), "ID2D1Factory3::CreateDevice");
	ThrowIfFailed(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContext), "CreateDeviceContext");
	d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(dwriteFactory.GetAddressOf())), "DWriteCreateFactory");
}

void Renderer::CreateSizeDependentResources() {
	for (UINT i = 0; i < kBackBufferCount; ++i) {
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i])), "GetBuffer");
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, RtvHandle(i));

		// 3D 패스가 끝나면 백버퍼는 RENDER_TARGET 상태, D2D가 끝나면 PRESENT로 돌려준다
		D3D11_RESOURCE_FLAGS flags = { D3D11_BIND_RENDER_TARGET };
		ThrowIfFailed(d3d11On12Device->CreateWrappedResource(backBuffers[i].Get(), &flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&wrappedBackBuffers[i])), "CreateWrappedResource");

		ComPtr<IDXGISurface> surface;
		ThrowIfFailed(wrappedBackBuffers[i].As(&surface), "IDXGISurface");
		D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(kBackBufferFormat, D2D1_ALPHA_MODE_PREMULTIPLIED), 96.0f, 96.0f);
		ThrowIfFailed(d2dContext->CreateBitmapFromDxgiSurface(surface.Get(), &props, &d2dTargets[i]), "CreateBitmapFromDxgiSurface");
	}

	D3D12_HEAP_PROPERTIES defaultHeap{};
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC texDesc{};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = sampleCount;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	if (sampleCount > 1) {
		texDesc.Format = kBackBufferFormat;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		D3D12_CLEAR_VALUE clear{};
		clear.Format = kBackBufferFormat;
		memcpy(clear.Color, kClearColor, sizeof(kClearColor));
		ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE, &clear, IID_PPV_ARGS(&msaaTarget)), "CreateCommittedResource(MSAA)");
		device->CreateRenderTargetView(msaaTarget.Get(), nullptr, RtvHandle(kBackBufferCount));
	}

	texDesc.Format = kDepthFormat;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE depthClear{};
	depthClear.Format = kDepthFormat;
	depthClear.DepthStencil.Depth = 1.0f;
	ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClear, IID_PPV_ARGS(&depthBuffer)), "CreateCommittedResource(depth)");
	device->CreateDepthStencilView(depthBuffer.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void Renderer::ReleaseSizeDependentResources() {
	d2dContext->SetTarget(nullptr);
	for (UINT i = 0; i < kBackBufferCount; ++i) {
		d2dTargets[i].Reset();
		wrappedBackBuffers[i].Reset();
		backBuffers[i].Reset();
	}
	msaaTarget.Reset();
	depthBuffer.Reset();
	d3d11Context->Flush();	// 11On12가 잡고 있던 백버퍼 참조를 실제로 해제
}

void Renderer::Resize(UINT w, UINT h) {
	if (!swapChain) return;
	if (w == 0 || h == 0) {
		minimized = true;
		return;
	}
	minimized = false;
	if (w == width && h == height) return;

	WaitForGpu();
	ReleaseSizeDependentResources();
	ThrowIfFailed(swapChain->ResizeBuffers(kBackBufferCount, w, h, kBackBufferFormat, 0), "ResizeBuffers");
	width = w;
	height = h;
	CreateSizeDependentResources();
}

void Renderer::Shutdown() {
	if (!device) return;
	WaitForGpu();
	ReleaseSizeDependentResources();
	if (fenceEvent) {
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::RtvHandle(UINT index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE h = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	h.ptr += static_cast<SIZE_T>(index) * rtvDescriptorSize;
	return h;
}

bool Renderer::BeginFrame(const FrameConstants& frame) {
	if (minimized) return false;

	memcpy(frameConstantsMapped, &frame, sizeof(frame));

	ThrowIfFailed(allocator->Reset(), "CommandAllocator::Reset");
	ThrowIfFailed(cmdList->Reset(allocator.Get(), pipelineState.Get()), "CommandList::Reset");

	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	if (sampleCount > 1) {
		auto b = Transition(msaaTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->ResourceBarrier(1, &b);
		rtv = RtvHandle(kBackBufferCount);
	}
	else {
		auto b = Transition(backBuffers[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->ResourceBarrier(1, &b);
		rtv = RtvHandle(frameIndex);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsvHeap->GetCPUDescriptorHandleForHeapStart();

	cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	cmdList->ClearRenderTargetView(rtv, kClearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	D3D12_RECT scissor{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissor);

	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, frameConstantBuffer->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
	cmdList->IASetIndexBuffer(&indexBufferView);
	return true;
}

void Renderer::DrawMesh(const MeshRange& mesh, const ObjectConstants& constants) {
	cmdList->SetGraphicsRoot32BitConstants(1, sizeof(ObjectConstants) / 4, &constants, 0);
	cmdList->DrawIndexedInstanced(mesh.indexCount, 1, mesh.startIndex, mesh.baseVertex, 0);
}

void Renderer::DrawTable() {
	ObjectConstants c{};
	XMStoreFloat4x4(&c.world, XMMatrixIdentity());
	c.color = { 0.11f, 0.42f, 0.27f, 1.0f };
	c.params = { 0.0f, 0.0f, 0.0f, 0.0f };
	DrawMesh(tableMesh, c);
}

void Renderer::DrawDie(const XMMATRIX& world, const XMFLOAT4& color, float highlight, float keep) {
	ObjectConstants c{};
	XMStoreFloat4x4(&c.world, XMMatrixTranspose(world));
	c.color = color;
	c.params = { 1.0f, highlight, keep, 0.0f };
	DrawMesh(dieMesh, c);
}

ID2D1DeviceContext2* Renderer::BeginUi() {
	ID3D12Resource* backBuffer = backBuffers[frameIndex].Get();
	if (sampleCount > 1) {
		D3D12_RESOURCE_BARRIER toResolve[2] = {
			Transition(msaaTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
			Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST),
		};
		cmdList->ResourceBarrier(2, toResolve);
		cmdList->ResolveSubresource(backBuffer, 0, msaaTarget.Get(), 0, kBackBufferFormat);
		auto toTarget = Transition(backBuffer, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->ResourceBarrier(1, &toTarget);
	}
	ThrowIfFailed(cmdList->Close(), "CommandList::Close");
	ID3D12CommandList* lists[] = { cmdList.Get() };
	queue->ExecuteCommandLists(1, lists);

	d3d11On12Device->AcquireWrappedResources(wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
	d2dContext->SetTarget(d2dTargets[frameIndex].Get());
	d2dContext->BeginDraw();
	d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
	return d2dContext.Get();
}

void Renderer::EndFrame() {
	d2dContext->EndDraw();
	d3d11On12Device->ReleaseWrappedResources(wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
	d3d11Context->Flush();

	swapChain->Present(1, 0);
	// 턴제 게임이라 프레임마다 GPU를 기다리는 단순한 동기화로 충분하다
	WaitForGpu();
	frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void Renderer::WaitForGpu() {
	if (!queue || !fence) return;
	++fenceValue;
	ThrowIfFailed(queue->Signal(fence.Get(), fenceValue), "Signal");
	if (fence->GetCompletedValue() < fenceValue) {
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent), "SetEventOnCompletion");
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}
